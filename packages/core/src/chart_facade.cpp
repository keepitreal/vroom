// Public C facade — the surface declared in `vroom/vroom_chart.h`.
//
// Most functions are thin wrappers over mutators on `VroomChart` plus
// `mark_dirty`. The actual rendering and animation logic lives in
// chart.cpp / labels.cpp / candles.cpp.

#include "vroom/vroom_chart.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "chart.h"
#include "labels.h"
#include "viewport.h"

class SkCanvas;

// ---- Sensitivity / clamping constants -------------------------------------

namespace {

// Pixels per 2× scale factor on axis-strip drags. Smaller = more aggressive.
constexpr double kAxisDragSensitivity = 300.0;

// Bounds for x-axis-drag clamp: minimum / maximum candle body width in pixels.
// The clamp converts these to corresponding time-window bounds based on the
// current candle area, so candles stay visually reasonable across zoom.
constexpr double kMinCandleBodyPx = 1.5;
constexpr double kMaxCandleBodyPx = 32.0;

}  // namespace

// ---- Lifecycle -------------------------------------------------------------

extern "C" VroomChart* vroom_chart_create(const VroomCallbacks* cb, void* user_ctx) {
    auto* c = new VroomChart();
    if (cb) c->cb = *cb;
    c->user_ctx = user_ctx;
    return c;
}

extern "C" void vroom_chart_destroy(VroomChart* chart) { delete chart; }

// ---- Data ------------------------------------------------------------------

extern "C" void vroom_chart_set_candles(VroomChart* chart, const VroomCandle* data, size_t count) {
    if (!chart) return;
    chart->candles.assign(data, data + count);
    chart->rsi_dirty = true;

    // Infer the candle period from the first interval. Robust enough for
    // uniform-duration series (the only kind we model today).
    if (chart->candles.size() >= 2) {
        const int64_t d = chart->candles[1].time_ms - chart->candles[0].time_ms;
        if (d > 0) chart->candle_duration_ms = d;
    }
    vroom::labels::recompute_axis_width(*chart);

    // Default the visible window to the most recent ~80 candles when the
    // consumer hasn't set one — a narrower x-window so candles read wider.
    if (chart->visible_start_ms == 0 && chart->visible_end_ms == 0 &&
        !chart->candles.empty()) {
        constexpr size_t kDefaultVisible = 80;
        const size_t start_idx = chart->candles.size() > kDefaultVisible
            ? chart->candles.size() - kDefaultVisible
            : 0;
        chart->visible_start_ms = chart->candles[start_idx].time_ms;
        chart->visible_end_ms = chart->candles.back().time_ms;
    }

    // Snap the price (y-axis) bounds to whatever's visible right now. After
    // this, only zoom / axis-drags change them; panning preserves them.
    if (!chart->candles.empty()) {
        const auto idx = vroom::visible_indices(
            chart->candles.data(), chart->candles.size(),
            chart->visible_start_ms, chart->visible_end_ms);
        if (idx.end > idx.start) {
            auto b = vroom::price_bounds(
                chart->candles.data() + idx.start, idx.end - idx.start);
            // Widen the default y-window beyond the data's min/max so candles
            // fill less vertical space (shorter candles, more headroom). 1.0 =
            // snug; larger = wider. Only the default — zoom/axis-drag override.
            constexpr double kDefaultYZoom = 1.5;
            const double mid = (b.min + b.max) * 0.5;
            const double half = (b.max - b.min) * 0.5 * kDefaultYZoom;
            b.min = mid - half;
            b.max = mid + half;
            chart->price_bounds = b;
            chart->price_bounds_initialized = true;
        }
    }

    chart->mark_dirty();
}

extern "C" void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c) return;
    chart->candles.push_back(*c);
    chart->rsi_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c || chart->candles.empty()) return;
    chart->candles.back() = *c;
    chart->rsi_dirty = true;
    chart->mark_dirty();
}

// ---- Layout ----------------------------------------------------------------

extern "C" void vroom_chart_set_size(VroomChart* chart, float w, float h, float ratio) {
    if (!chart) return;
    if (chart->width_px == w && chart->height_px == h && chart->px_ratio == ratio) return;
    chart->width_px = w;
    chart->height_px = h;
    chart->px_ratio = ratio;
    chart->mark_dirty();
}

// ---- Viewport / gestures ---------------------------------------------------

extern "C" void vroom_chart_set_visible_range(VroomChart* chart, int64_t start_ms, int64_t end_ms) {
    if (!chart) return;
    if (chart->visible_start_ms == start_ms && chart->visible_end_ms == end_ms) return;
    chart->visible_start_ms = start_ms;
    chart->visible_end_ms = end_ms;
    if (chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(chart->user_ctx, start_ms, end_ms);
    }
    chart->mark_dirty();
}

extern "C" void vroom_chart_pan(VroomChart* chart, float dx_px, float /*dy_px*/) {
    if (!chart || dx_px == 0.f || chart->candles.empty()) return;

    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    const float usable_px =
        chart->width_px - chart->theme.floats[VROOM_FLOAT_RIGHT_PADDING_PX];
    if (window_ms <= 0 || usable_px <= 0.f) return;

    // Finger right (dx > 0) → content moves right → see earlier data → window shifts back.
    const int64_t delta_ms = static_cast<int64_t>(
        (-dx_px / usable_px) * static_cast<float>(window_ms));
    if (delta_ms == 0) return;

    int64_t new_start = chart->visible_start_ms + delta_ms;
    int64_t new_end = chart->visible_end_ms + delta_ms;

    // Right edge can overshoot the last candle by up to half a window so the
    // user can scroll into empty "future" space (e.g., to view right-anchored
    // indicators). Left edge still hard-clamps at the first candle.
    const int64_t first_time = chart->candles.front().time_ms;
    const int64_t last_time = chart->candles.back().time_ms;
    const int64_t max_future = window_ms / 2;
    if (new_end > last_time + max_future) {
        new_end = last_time + max_future;
        new_start = new_end - window_ms;
    }
    if (new_start < first_time) {
        new_start = first_time;
        new_end = new_start + window_ms;
    }

    if (new_start == chart->visible_start_ms &&
        new_end == chart->visible_end_ms) {
        return;  // fully clamped, nothing changed
    }

    chart->visible_start_ms = new_start;
    chart->visible_end_ms = new_end;

    if (chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(
            chart->user_ctx, chart->visible_start_ms, chart->visible_end_ms);
    }
    chart->mark_dirty();
}

extern "C" void vroom_chart_translate(VroomChart* chart, float dx_px, float dy_px) {
    if (!chart || chart->candles.empty()) return;
    bool changed = false;

    // Horizontal: shift the time window. Same math + clamping as pan, but
    // we don't fire the redraw notification mid-call — we batch it below.
    if (dx_px != 0.f) {
        const int64_t window_ms =
            chart->visible_end_ms - chart->visible_start_ms;
        const float usable_px =
            chart->width_px - chart->theme.floats[VROOM_FLOAT_RIGHT_PADDING_PX];
        if (window_ms > 0 && usable_px > 0.f) {
            const int64_t delta_ms = static_cast<int64_t>(
                (-dx_px / usable_px) * static_cast<float>(window_ms));
            if (delta_ms != 0) {
                int64_t new_start = chart->visible_start_ms + delta_ms;
                int64_t new_end = chart->visible_end_ms + delta_ms;
                const int64_t first_time = chart->candles.front().time_ms;
                const int64_t last_time = chart->candles.back().time_ms;
                const int64_t max_future = window_ms / 2;
                if (new_end > last_time + max_future) {
                    new_end = last_time + max_future;
                    new_start = new_end - window_ms;
                }
                if (new_start < first_time) {
                    new_start = first_time;
                    new_end = new_start + window_ms;
                }
                if (new_start != chart->visible_start_ms ||
                    new_end != chart->visible_end_ms) {
                    chart->visible_start_ms = new_start;
                    chart->visible_end_ms = new_end;
                    changed = true;
                }
            }
        }
    }

    // Vertical: shift price bounds by the price-equivalent of dy_px. Range
    // stays constant. We divide by draw_h (the 90% slice of candle area
    // that's actually used for price → y) so translation feels 1:1.
    if (dy_px != 0.f && chart->price_bounds_initialized) {
        const float candle_area_h = vroom::price_pane_bottom(chart->layout());
        const double draw_h = static_cast<double>(candle_area_h) * 0.9;
        if (draw_h > 0.0) {
            const double range =
                chart->price_bounds.max - chart->price_bounds.min;
            const double dprice =
                (static_cast<double>(dy_px) / draw_h) * range;
            chart->price_bounds.min += dprice;
            chart->price_bounds.max += dprice;
            vroom::labels::recompute_axis_width(*chart);
            changed = true;
        }
    }

    if (changed) {
        if (chart->cb.on_viewport_changed) {
            chart->cb.on_viewport_changed(
                chart->user_ctx, chart->visible_start_ms,
                chart->visible_end_ms);
        }
        chart->mark_dirty();
    }
}

extern "C" void vroom_chart_scale_price_axis(VroomChart* chart, float dy_px) {
    if (!chart || dy_px == 0.f) return;
    if (!chart->price_bounds_initialized) return;

    const double range = chart->price_bounds.max - chart->price_bounds.min;
    if (range <= 0.0) return;

    // Drag down (dy > 0) → scale > 1 → wider price range → candles shrink.
    double scale = 1.0 + static_cast<double>(dy_px) / kAxisDragSensitivity;
    if (scale < 0.05) scale = 0.05;  // never collapse or flip

    const double center = (chart->price_bounds.max + chart->price_bounds.min) * 0.5;
    const double new_range = range * scale;
    chart->price_bounds.max = center + new_range * 0.5;
    chart->price_bounds.min = center - new_range * 0.5;
    vroom::labels::recompute_axis_width(*chart);

    chart->mark_dirty();
}

extern "C" void vroom_chart_scale_time_axis(VroomChart* chart, float dx_px) {
    if (!chart || dx_px == 0.f || chart->candles.empty()) return;

    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (window_ms <= 0) return;

    // Drag right (dx > 0) → scale > 1 → wider time window → candles thin.
    double scale = 1.0 + static_cast<double>(dx_px) / kAxisDragSensitivity;
    if (scale < 0.05) scale = 0.05;

    int64_t new_window_ms = static_cast<int64_t>(
        static_cast<double>(window_ms) * scale);

    // Clamp window so candle body width stays in [kMinCandleBodyPx,
    // kMaxCandleBodyPx]. body_w = usable × (dur / window) × ratio →
    // window for a given body_w = usable × dur × ratio / body_w.
    const auto lay = chart->layout();
    const double usable =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    const double ratio = chart->theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO];
    const double dur = static_cast<double>(chart->candle_duration_ms);
    if (usable > 0.0 && ratio > 0.0 && dur > 0.0) {
        const int64_t min_window = static_cast<int64_t>(
            (usable * dur * ratio) / kMaxCandleBodyPx);
        const int64_t max_window = static_cast<int64_t>(
            (usable * dur * ratio) / kMinCandleBodyPx);
        if (new_window_ms < min_window) new_window_ms = min_window;
        if (new_window_ms > max_window) new_window_ms = max_window;
    }

    // Pivot around the right edge — most-recent visible candle stays put.
    int64_t new_start = chart->visible_end_ms - new_window_ms;
    const int64_t first_time = chart->candles.front().time_ms;
    if (new_start < first_time) new_start = first_time;

    if (new_start == chart->visible_start_ms) return;
    chart->visible_start_ms = new_start;

    if (chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(
            chart->user_ctx, chart->visible_start_ms, chart->visible_end_ms);
    }
    chart->mark_dirty();
}

extern "C" void vroom_chart_get_axis_metrics(VroomChart* chart,
                                              float* out_y_axis_width_px,
                                              float* out_x_axis_height_px,
                                              float* out_indicator_height_px) {
    if (!chart) return;
    const auto lay = chart->layout();
    if (out_y_axis_width_px) *out_y_axis_width_px = lay.y_axis_width_px;
    if (out_x_axis_height_px) *out_x_axis_height_px = lay.x_axis_height_px;
    if (out_indicator_height_px) *out_indicator_height_px = lay.indicator_area_h;
}

extern "C" void vroom_chart_zoom(VroomChart* chart, float scale_x, float scale_y,
                                 float fx, float fy) {
    if (!chart) return;
    bool price_changed = false;
    bool window_changed = false;

    // --- Y (price) zoom around fy ------------------------------------------
    // scale_y > 1 (vertical pinch out) narrows the price range → taller candles.
    if (scale_y > 0.f && scale_y != 1.f && chart->price_bounds_initialized) {
        const float candle_area_h = vroom::price_pane_bottom(chart->layout());
        const double range = chart->price_bounds.max - chart->price_bounds.min;
        if (candle_area_h > 0.f && range > 0.0) {
            const float frac = std::clamp(fy / candle_area_h, 0.f, 1.f);
            const double focal_price = chart->price_bounds.max - frac * range;
            const double new_range = range / static_cast<double>(scale_y);
            chart->price_bounds.max = focal_price + frac * new_range;
            chart->price_bounds.min = chart->price_bounds.max - new_range;
            price_changed = true;
        }
    }

    // --- X (time) zoom around fx -------------------------------------------
    // scale_x > 1 (horizontal pinch out) narrows the time window → wider candles.
    if (scale_x > 0.f && scale_x != 1.f && !chart->candles.empty()) {
        const int64_t window_ms =
            chart->visible_end_ms - chart->visible_start_ms;
        const auto lay = chart->layout();
        const double usable =
            lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
        if (window_ms > 0 && usable > 0.0) {
            const double fracX =
                std::clamp(static_cast<double>(fx) / usable, 0.0, 1.0);
            const int64_t focal_time = chart->visible_start_ms +
                static_cast<int64_t>(fracX * static_cast<double>(window_ms));

            int64_t new_window = static_cast<int64_t>(
                static_cast<double>(window_ms) / static_cast<double>(scale_x));

            // Keep candle body width within [min, max] (same bounds as the
            // x-axis drag): window = usable × dur × ratio / body_w.
            const double ratio =
                chart->theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO];
            const double dur = static_cast<double>(chart->candle_duration_ms);
            if (ratio > 0.0 && dur > 0.0) {
                const int64_t min_window = static_cast<int64_t>(
                    (usable * dur * ratio) / kMaxCandleBodyPx);
                const int64_t max_window = static_cast<int64_t>(
                    (usable * dur * ratio) / kMinCandleBodyPx);
                if (new_window < min_window) new_window = min_window;
                if (new_window > max_window) new_window = max_window;
            }

            if (new_window > 0) {
                int64_t new_start = focal_time -
                    static_cast<int64_t>(fracX * static_cast<double>(new_window));
                int64_t new_end = new_start + new_window;
                const int64_t first_time = chart->candles.front().time_ms;
                const int64_t last_time = chart->candles.back().time_ms;
                const int64_t max_future = new_window / 2;
                if (new_end > last_time + max_future) {
                    new_end = last_time + max_future;
                    new_start = new_end - new_window;
                }
                if (new_start < first_time) {
                    new_start = first_time;
                    new_end = new_start + new_window;
                }
                if (new_start != chart->visible_start_ms ||
                    new_end != chart->visible_end_ms) {
                    chart->visible_start_ms = new_start;
                    chart->visible_end_ms = new_end;
                    window_changed = true;
                }
            }
        }
    }

    if (price_changed) vroom::labels::recompute_axis_width(*chart);
    if (window_changed && chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(
            chart->user_ctx, chart->visible_start_ms, chart->visible_end_ms);
    }
    if (price_changed || window_changed) chart->mark_dirty();
}

// ---- Crosshair (state only; rendering not yet implemented) ----------------

extern "C" void vroom_chart_set_crosshair(VroomChart* chart, float x, float y) {
    if (!chart) return;
    chart->crosshair_active = true;
    chart->crosshair_x_px = x;
    chart->crosshair_y_px = y;
    chart->mark_dirty();
}

extern "C" void vroom_chart_clear_crosshair(VroomChart* chart) {
    if (!chart) return;
    chart->crosshair_active = false;
    chart->mark_dirty();
}

extern "C" bool vroom_chart_get_crosshair_candle(VroomChart* chart,
                                                 VroomCandle* out) {
    if (!chart || !out || !chart->crosshair_active || chart->candles.empty()) {
        return false;
    }
    // Mirror draw_chart's visible-slice derivation so the queried candle is
    // exactly the one the crosshair snaps to on screen.
    const auto lay = chart->layout();
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (n == 0 || window_ms <= 0) return false;

    const ::VroomCandle* visible = chart->candles.data() + range.start;
    const size_t idx = vroom::snap_index_to_candle(
        lay, visible, n, chart->candle_duration_ms,
        chart->visible_start_ms, window_ms, chart->crosshair_x_px);
    *out = visible[idx];
    return true;
}

// ---- Indicators -----------------------------------------------------------

extern "C" void vroom_chart_set_rsi(VroomChart* chart, bool enabled, int period) {
    if (!chart) return;
    if (period < 2) period = 2;
    if (chart->rsi_enabled == enabled && chart->rsi_period == period) return;
    chart->rsi_enabled = enabled;
    chart->rsi_period = period;
    chart->rsi_dirty = true;
    chart->mark_dirty();
}

// ---- Direct draw (used by hosts that don't need the SkPicture cache) ------

extern "C" void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas) {
    if (!chart || !canvas) return;
    chart->draw_chart(canvas);
}

// ---- Theme setters ---------------------------------------------------------

extern "C" void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb) {
    if (!chart || key < 0 || key >= VROOM_COLOR_COUNT_) return;
    if (chart->theme.colors[key] == argb) return;
    chart->theme.colors[key] = argb;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value) {
    if (!chart || key < 0 || key >= VROOM_FLOAT_COUNT_) return;
    if (chart->theme.floats[key] == value) return;
    chart->theme.floats[key] = value;
    if (key == VROOM_FLOAT_AXIS_FONT_SIZE_PX) {
        vroom::labels::recompute_axis_width(*chart);
    }
    chart->mark_dirty();
}
