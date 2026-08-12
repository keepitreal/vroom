// Public C facade — the surface declared in `vroom/vroom_chart.h`.
//
// Most functions are thin wrappers over mutators on `VroomChart` plus
// `mark_dirty`. The actual rendering and animation logic lives in
// chart.cpp / labels.cpp / candles.cpp.

#include "vroom/vroom_chart.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "chart.h"
#include "drawings.h"
#include "labels.h"
#include "price_lines.h"
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

// Hard cap on the future gap (visible_end - last_candle): 3/4 of the window so
// at least 25% of the chart still shows candles. Never forces an existing larger
// gap (e.g. from the x-axis drag zoom) to snap back — only limits new growth.
int64_t max_future_gap(int64_t window_ms, int64_t cur_future) {
    return std::max<int64_t>((window_ms * 3) / 4, cur_future);
}

// Rubber-band a forward (into-future) pan delta: once the current future gap is
// past the soft cap (window/2), damp the delta quadratically toward zero as it
// nears the hard cap (3*window/4). Backward (into-past) deltas are unaffected.
int64_t damp_future_delta(int64_t delta_ms, int64_t cur_future,
                          int64_t window_ms) {
    if (delta_ms <= 0) return delta_ms;       // only resist forward motion
    const int64_t soft = window_ms / 2;
    if (cur_future <= soft) return delta_ms;  // free until the soft cap
    const int64_t hard = (window_ms * 3) / 4;
    const double span = static_cast<double>(hard - soft);
    if (span <= 0.0) return 0;
    const double depth = std::clamp(
        static_cast<double>(cur_future - soft) / span, 0.0, 1.0);
    const double resist = (1.0 - depth) * (1.0 - depth);
    return static_cast<int64_t>(static_cast<double>(delta_ms) * resist);
}

// Frame the default view. When the consumer has set a target candle body width
// (default_candle_px), size the window so each candle renders ~that wide with the
// right edge pinned to the latest candle; otherwise fall back to the legacy
// "most recent ~80 candles". No-op when there are no candles.
void apply_default_framing(VroomChart* chart) {
    if (chart->candles.empty()) return;

    // A candle occupies the slot [time_ms, time_ms + duration) and draws centered
    // in it, so the right edge of the view is the last slot's *end*. Pinning it to
    // the last candle's time_ms instead puts that candle's center half a slot past
    // the plot area, hiding the newest candle behind the y-axis strip.
    const int64_t right_edge_ms =
        chart->candles.back().time_ms + chart->candle_duration_ms;

    // Px-driven framing. Reuses the inversion from vroom_chart_scale_time_axis:
    // body_w = usable × (dur / window) × ratio  →  window = usable × dur × ratio / body_w.
    // Needs a valid layout (width_px); until size is known it falls through to
    // the candle-count default below.
    if (chart->default_candle_px > 0.f) {
        const auto lay = chart->layout();
        const double usable =
            lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
        const double ratio = chart->theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO];
        const double dur = static_cast<double>(chart->candle_duration_ms);
        const double body = std::clamp(static_cast<double>(chart->default_candle_px),
                                       kMinCandleBodyPx, kMaxCandleBodyPx);
        if (usable > 0.0 && ratio > 0.0 && dur > 0.0 && body > 0.0) {
            int64_t window_ms = static_cast<int64_t>((usable * dur * ratio) / body);
            const int64_t span =
                chart->candles.back().time_ms - chart->candles.front().time_ms;
            window_ms = std::clamp<int64_t>(window_ms, chart->candle_duration_ms,
                                            span > 0 ? span : window_ms);
            chart->visible_end_ms = right_edge_ms;
            chart->visible_start_ms = chart->visible_end_ms - window_ms;
            return;
        }
    }

    constexpr size_t kDefaultVisible = 80;
    const size_t start_idx = chart->candles.size() > kDefaultVisible
        ? chart->candles.size() - kDefaultVisible
        : 0;
    chart->visible_start_ms = chart->candles[start_idx].time_ms;
    chart->visible_end_ms = right_edge_ms;
}

// On the first manual-y gesture, adopt the on-screen auto-fit bounds so the
// gesture continues from exactly what the user sees. No-op once manual.
void ensure_manual_price_bounds(VroomChart* chart) {
    if (chart->price_bounds_manual) return;
    const auto idx = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    if (idx.end > idx.start) {
        chart->price_bounds = vroom::auto_price_bounds(
            chart->candles.data() + idx.start, idx.end - idx.start);
    }  // else: no visible candles — keep whatever bounds we had
    chart->price_bounds_manual = true;
    vroom::labels::recompute_axis_width(*chart);
}

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
    chart->macd_dirty = true;
    chart->overlays_dirty = true;
    chart->vwap_dirty = true;
    chart->bollinger_dirty = true;

    // Infer the candle period from the first interval. Robust enough for
    // uniform-duration series (the only kind we model today).
    if (chart->candles.size() >= 2) {
        const int64_t d = chart->candles[1].time_ms - chart->candles[0].time_ms;
        if (d > 0) chart->candle_duration_ms = d;
    }
    vroom::labels::recompute_axis_width(*chart);

    // Default the visible window when the consumer hasn't set one. The y-axis
    // needs no equivalent: while the price scale is in auto mode the draw path
    // fits it to the visible candles every frame, and once the user has taken
    // it manual, live data updates must not disturb their pan/scale.
    if (chart->visible_start_ms == 0 && chart->visible_end_ms == 0) {
        apply_default_framing(chart);
    }

    chart->mark_dirty();
}

extern "C" void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c) return;
    chart->candles.push_back(*c);
    chart->rsi_dirty = true;
    chart->macd_dirty = true;
    chart->overlays_dirty = true;
    chart->vwap_dirty = true;
    chart->bollinger_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c || chart->candles.empty()) return;
    chart->candles.back() = *c;
    chart->rsi_dirty = true;
    chart->macd_dirty = true;
    chart->overlays_dirty = true;
    chart->vwap_dirty = true;
    chart->bollinger_dirty = true;
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

extern "C" void vroom_chart_set_default_candle_width(VroomChart* chart, float px) {
    if (!chart) return;
    chart->default_candle_px = px;
    // Re-frame so a value that arrives after set_candles still drives the initial
    // view. Intended for initial framing only — calling it after the user has
    // panned would snap the view back to the default frame.
    if (!chart->candles.empty()) apply_default_framing(chart);
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_chart_type(VroomChart* chart, int32_t mode) {
    if (!chart) return;
    chart->chart_type = mode;
    // Snap the morph blend to the target so a direct set (no animation) renders
    // the requested mode immediately. The JS animation loop uses set_morph for
    // the in-between frames and then calls this to lock the final state.
    const float t = mode == 1 ? 1.f : 0.f;
    chart->morph_collapse = t;
    chart->morph_fade = t;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_morph(VroomChart* chart, float collapse,
                                      float fade) {
    if (!chart) return;
    chart->morph_collapse = collapse;
    chart->morph_fade = fade;
    chart->mark_dirty();
}

extern "C" void vroom_chart_get_visible_range(VroomChart* chart,
                                              int64_t* out_start_ms,
                                              int64_t* out_end_ms) {
    if (!chart) return;
    if (out_start_ms) *out_start_ms = chart->visible_start_ms;
    if (out_end_ms) *out_end_ms = chart->visible_end_ms;
}

extern "C" void vroom_chart_reset_view(VroomChart* chart) {
    if (!chart) return;
    chart->visible_start_ms = 0;
    chart->visible_end_ms = 0;
    chart->price_bounds_manual = false;
    // An asset switch reframes wholesale, so a slot-paired morph is meaningless
    // here — drop any capture rather than leaving it to reshape the wrong data.
    chart->morph_from.clear();
    chart->interval_morph_t = 1.f;
    apply_default_framing(chart);
    vroom::labels::recompute_axis_width(*chart);
    if (chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(chart->user_ctx, chart->visible_start_ms,
                                      chart->visible_end_ms);
    }
    chart->mark_dirty();
}

extern "C" void vroom_chart_reset_price_scale(VroomChart* chart) {
    if (!chart || !chart->price_bounds_manual) return;
    chart->price_bounds_manual = false;
    vroom::labels::recompute_axis_width(*chart);
    chart->mark_dirty();
}

extern "C" bool vroom_chart_get_visible_price_envelope(VroomChart* chart,
                                                       double* out_low,
                                                       double* out_high) {
    if (!chart) return false;
    const auto idx = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    if (idx.end <= idx.start) return false;
    const auto env = vroom::price_bounds(chart->candles.data() + idx.start,
                                         idx.end - idx.start);
    if (out_low) *out_low = env.min;
    if (out_high) *out_high = env.max;
    return true;
}

extern "C" void vroom_chart_preserve_price_envelope(VroomChart* chart,
                                                    double prev_low,
                                                    double prev_high) {
    // Auto mode already holds the envelope's pixel height constant (auto_price_
    // bounds widens it by a fixed factor), so there is nothing to preserve.
    if (!chart || !chart->price_bounds_manual) return;

    double new_low = 0.0, new_high = 0.0;
    const bool has_new =
        vroom_chart_get_visible_price_envelope(chart, &new_low, &new_high);
    if (!has_new || !(prev_high > prev_low) || !(new_high > new_low)) {
        // Nothing to scale against — fall back to re-fitting the price scale.
        vroom_chart_reset_price_scale(chart);
        return;
    }

    chart->price_bounds = vroom::preserve_envelope_bounds(
        chart->price_bounds, {prev_low, prev_high}, {new_low, new_high});
    vroom::labels::recompute_axis_width(*chart);
    chart->mark_dirty();
}

extern "C" void vroom_chart_begin_interval_morph(VroomChart* chart) {
    if (!chart) return;
    chart->morph_from.clear();
    chart->interval_morph_t = 1.f;

    const auto lay = chart->layout();
    const float area_w = vroom::candle_area_width(lay);
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (area_w <= 0.f || window_ms <= 0) return;

    const auto idx = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const std::size_t n = idx.end - idx.start;
    if (n == 0) return;
    const ::VroomCandle* visible = chart->candles.data() + idx.start;

    const auto bounds = chart->price_bounds_manual
        ? chart->price_bounds
        : vroom::auto_price_bounds(visible, n);

    // Normalized so the capture is independent of both the price bounds (which
    // the swap is about to replace) and the surface size (which may change
    // mid-morph). Newest candle first, matching the slot indexing in
    // candles::draw.
    chart->morph_from.resize(n);
    for (std::size_t k = 0; k < n; ++k) {
        const auto& c = visible[n - 1 - k];
        chart->morph_from[k] = vroom::CandleSnapshot{
            vroom::candle_center_x(lay, c.time_ms, chart->candle_duration_ms,
                                   chart->visible_start_ms, window_ms) / area_w,
            static_cast<float>(vroom::price_fraction(bounds, c.open)),
            static_cast<float>(vroom::price_fraction(bounds, c.high)),
            static_cast<float>(vroom::price_fraction(bounds, c.low)),
            static_cast<float>(vroom::price_fraction(bounds, c.close)),
            c.close >= c.open,
        };
    }

    // The scale the capture was taken against — the axes render their outgoing
    // ticks from it, and it's about to be replaced on the chart itself.
    chart->morph_from_bounds = bounds;
    chart->morph_from_start_ms = chart->visible_start_ms;
    chart->morph_from_end_ms = chart->visible_end_ms;

    // Open the morph at 0 rather than leaving it at 1: the caller still has to
    // push the new candles, and any frame painted in between should show the
    // captured geometry — which is what the pre-switch frame looked like.
    chart->interval_morph_t = 0.f;
}

extern "C" void vroom_chart_set_interval_morph(VroomChart* chart, float t) {
    if (!chart) return;
    chart->interval_morph_t = std::clamp(t, 0.f, 1.f);
    if (chart->interval_morph_t >= 1.f) {
        chart->morph_from.clear();
        chart->morph_from.shrink_to_fit();
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

    // The right edge can overshoot the last candle into empty "future" space.
    // Free scrolling up to half a window (candles fill >= 50%); from there to
    // 3/4 of a window (candles fill >= 25%) panning forward is rubber-banded so
    // it slows and hard-stops at the cap. The x-axis drag zoom can leave a gap
    // larger than the cap (it zooms in until ~one candle remains), so the cap
    // never forces it back — it just shrinks naturally as the user scrolls.
    // Left edge still hard-clamps at the first candle.
    const int64_t first_time = chart->candles.front().time_ms;
    const int64_t last_time = chart->candles.back().time_ms;
    const int64_t cur_future = chart->visible_end_ms - last_time;
    const int64_t eff_delta = damp_future_delta(delta_ms, cur_future, window_ms);

    int64_t new_start = chart->visible_start_ms + eff_delta;
    int64_t new_end = chart->visible_end_ms + eff_delta;

    const int64_t max_future = max_future_gap(window_ms, cur_future);
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
                // See vroom_chart_pan: rubber-band forward scrolling between the
                // 50% and 25%-candles caps, and let an existing larger gap (from
                // the x-axis drag zoom) shrink naturally instead of snapping.
                const int64_t first_time = chart->candles.front().time_ms;
                const int64_t last_time = chart->candles.back().time_ms;
                const int64_t cur_future = chart->visible_end_ms - last_time;
                const int64_t eff_delta =
                    damp_future_delta(delta_ms, cur_future, window_ms);
                int64_t new_start = chart->visible_start_ms + eff_delta;
                int64_t new_end = chart->visible_end_ms + eff_delta;
                const int64_t max_future =
                    max_future_gap(window_ms, cur_future);
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
    if (dy_px != 0.f && !chart->candles.empty()) {
        ensure_manual_price_bounds(chart);
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
    ensure_manual_price_bounds(chart);

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

    // Candle body width bounds [kMinCandleBodyPx, kMaxCandleBodyPx] map to a
    // window range: body_w = usable × (dur / window) × ratio →
    // window for a given body_w = usable × dur × ratio / body_w.
    const auto lay = chart->layout();
    const double usable =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    const double ratio = chart->theme.floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO];
    const double dur = static_cast<double>(chart->candle_duration_ms);
    int64_t min_window = 0;
    int64_t max_window = std::numeric_limits<int64_t>::max();
    if (usable > 0.0 && ratio > 0.0 && dur > 0.0) {
        min_window = static_cast<int64_t>((usable * dur * ratio) / kMaxCandleBodyPx);
        max_window = static_cast<int64_t>((usable * dur * ratio) / kMinCandleBodyPx);
    }

    // The right edge is pinned, so when it sits in the empty "future" space
    // past the last candle (pan/scroll allow it to overshoot by up to half a
    // window), zooming in shrinks the window from the left and slides candles
    // off-screen. Rather than blocking it outright, allow the zoom down to a
    // floor that keeps the last candle on screen, and add rubber-band
    // resistance once the future region passes 50% of the window so the
    // discouraged zoom visibly slows before it stops.
    const int64_t last_time = chart->candles.back().time_ms;
    const int64_t future_ms = chart->visible_end_ms - last_time;
    if (future_ms > 0 && new_window_ms < window_ms) {
        const int64_t floor_window = future_ms + chart->candle_duration_ms;
        const int64_t soft_window = 2 * future_ms;  // future occupies 50%
        if (soft_window > floor_window) {
            // depth 0 at the soft point, 1 at the floor; resistance eases the
            // shrink toward zero as the window approaches the floor.
            const double span = static_cast<double>(soft_window - floor_window);
            double depth = (static_cast<double>(soft_window) -
                            static_cast<double>(window_ms)) / span;
            depth = std::clamp(depth, 0.0, 1.0);
            const double resist = (1.0 - depth) * (1.0 - depth);
            const int64_t raw_delta = new_window_ms - window_ms;  // negative
            new_window_ms = window_ms + static_cast<int64_t>(
                static_cast<double>(raw_delta) * resist);
        }
        if (new_window_ms < floor_window) new_window_ms = floor_window;
    }

    if (new_window_ms < min_window) new_window_ms = min_window;
    if (new_window_ms > max_window) new_window_ms = max_window;

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

extern "C" void vroom_chart_resize_indicator_pane(VroomChart* chart, float dy_px) {
    if (!chart || dy_px == 0.f) return;

    const int pane_count = (chart->rsi.enabled ? 1 : 0) + (chart->macd.enabled ? 1 : 0);
    if (pane_count == 0) return;  // nothing below the chart to resize

    const auto lay = chart->layout();
    const float old_pane_bottom = vroom::price_pane_bottom(lay);
    const float old_band = lay.indicator_area_h;

    // Drag down (dy > 0) shrinks the indicator band and grows the price pane.
    // Keep both regions usable: each pane >= kMinPanePx, price pane >= a share
    // of the candle+indicator area.
    constexpr float kMinPanePx = 48.f;
    const float content_h = lay.height_px - lay.x_axis_height_px;  // candle + indicators
    const float min_band = static_cast<float>(pane_count) * kMinPanePx;
    const float max_band = content_h * 0.70f;  // leave the price pane >= 30%
    if (max_band <= min_band) return;

    float new_band = old_band - dy_px;
    new_band = std::clamp(new_band, min_band, max_band);
    if (new_band == old_band) return;

    const float per_pane = new_band / static_cast<float>(pane_count);
    chart->theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC] =
        chart->height_px > 0.f ? per_pane / chart->height_px : 0.f;

    // Preserve candle pixel scale: scale the price range by the price-pane
    // height ratio, anchoring the top (max) price so existing candles stay put
    // and the newly revealed space opens at the bottom.
    if (chart->price_bounds_manual && old_pane_bottom > 0.f) {
        const float new_pane_bottom = content_h - new_band;
        const double scale = static_cast<double>(new_pane_bottom) /
                             static_cast<double>(old_pane_bottom);
        const double range = chart->price_bounds.max - chart->price_bounds.min;
        if (range > 0.0 && scale > 0.0) {
            chart->price_bounds.min =
                chart->price_bounds.max - range * scale;
            vroom::labels::recompute_axis_width(*chart);
        }
    }

    chart->mark_dirty();
}

extern "C" void vroom_chart_scale_indicator_axis(VroomChart* chart, float y_px,
                                                  float dy_px) {
    if (!chart || dy_px == 0.f) return;

    const auto lay = chart->layout();
    if (lay.indicator_area_h <= 0.f) return;

    // Rebuild the same ordered pane stack draw_chart uses (most recently
    // enabled pane sorts to the bottom) to find which pane y_px falls in.
    struct ActivePane { int order; int type; };  // type: 0 = RSI, 1 = MACD
    ActivePane panes[2];
    int count = 0;
    if (chart->rsi.enabled) panes[count++] = {chart->rsi_order, 0};
    if (chart->macd.enabled) panes[count++] = {chart->macd_order, 1};
    if (count == 0) return;
    if (count == 2 && panes[0].order > panes[1].order) {
        const ActivePane tmp = panes[0];
        panes[0] = panes[1];
        panes[1] = tmp;
    }

    const float pane_h =
        chart->height_px * chart->theme.floats[VROOM_FLOAT_INDICATOR_HEIGHT_FRAC];
    if (pane_h <= 0.f) return;

    int target = -1;  // 0 = RSI, 1 = MACD
    float pane_top = vroom::price_pane_bottom(lay);
    for (int i = 0; i < count; ++i) {
        const float pane_bottom = pane_top + pane_h;
        if (y_px >= pane_top && y_px < pane_bottom) {
            target = panes[i].type;
            break;
        }
        pane_top = pane_bottom;
    }
    if (target < 0) return;  // not over a pane

    // Drag down (dy > 0) widens the visible value range (zoom out), matching
    // scale_price_axis's sign. The zoom is the inverse of the range scale.
    double range_scale = 1.0 + static_cast<double>(dy_px) / kAxisDragSensitivity;
    if (range_scale < 0.05) range_scale = 0.05;  // never collapse or flip

    double& zoom = target == 0 ? chart->rsi_y_scale : chart->macd_y_scale;
    double next = zoom / range_scale;
    next = std::clamp(next, 0.1, 10.0);
    if (next == zoom) return;
    zoom = next;

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
    if (scale_y > 0.f && scale_y != 1.f && !chart->candles.empty()) {
        ensure_manual_price_bounds(chart);
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
                // Cap the future gap at 3/4 of the (new) window so >= 25% of the
                // chart still shows candles, without snapping back an existing
                // larger gap (e.g. from the x-axis drag zoom).
                const int64_t max_future = max_future_gap(
                    new_window, chart->visible_end_ms - last_time);
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

extern "C" void vroom_chart_set_crosshair_data(VroomChart* chart,
                                               int64_t time_ms, double price) {
    if (!chart || chart->candles.empty()) return;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (window_ms <= 0) return;
    const auto lay = chart->layout();
    // Mirror coord_at's bounds so price -> y matches what's on screen.
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(chart->candles.data() + range.start, n);
    chart->crosshair_active = true;
    chart->crosshair_x_px =
        vroom::x_at_time(lay, chart->visible_start_ms, window_ms, time_ms);
    chart->crosshair_y_px = vroom::price_to_y(lay, bounds, price);
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

extern "C" bool vroom_chart_get_crosshair_info(VroomChart* chart,
                                               VroomCrosshairInfo* out) {
    if (!chart || !out || !chart->crosshair_active || chart->candles.empty()) {
        return false;
    }
    // Mirror draw_chart's visible-slice derivation so the queried slot is
    // exactly the one the crosshair snaps to on screen.
    const auto lay = chart->layout();
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (n == 0 || window_ms <= 0) return false;

    const ::VroomCandle* visible = chart->candles.data() + range.start;
    const vroom::SnapResult snap = vroom::snap_to_slot(
        lay, visible, n, chart->candle_duration_ms,
        chart->visible_start_ms, window_ms, chart->crosshair_x_px);
    out->time_ms = snap.time_ms;
    // Free price at the crosshair's horizontal line — same bounds as the price
    // badge (crosshair.cpp) so the reported value equals what's drawn.
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(visible, n);
    out->price = vroom::y_to_price(lay, bounds, chart->crosshair_y_px);
    out->has_candle = snap.has_candle;
    if (snap.has_candle) out->candle = visible[snap.index];
    return true;
}

// ---- Drawings (line annotations) ------------------------------------------

extern "C" void vroom_chart_set_drawings(VroomChart* chart,
                                         const VroomDrawing* drawings,
                                         size_t count) {
    if (!chart) return;
    // Deep-copy into the owning form: a pencil's path lives in the chart, so the
    // caller's `points` buffer may be freed as soon as this returns.
    chart->drawings.clear();
    chart->drawings.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const VroomDrawing& src = drawings[i];
        VroomChart::StoredDrawing d;
        d.a = src.a;
        d.b = src.b;
        d.color = src.color;
        d.width = src.width;
        d.kind = src.kind;
        if (src.kind == 2 && src.points && src.point_count > 0) {
            d.points.assign(src.points, src.points + src.point_count);
            // Keep a/b mirroring the path ends so bounds/handle code is uniform.
            d.a = d.points.front();
            d.b = d.points.back();
        }
        chart->drawings.push_back(std::move(d));
    }
    // Drop selection if the new set no longer contains that index.
    if (chart->selected_drawing >= static_cast<int32_t>(count)) {
        chart->selected_drawing = -1;
        chart->grabbed_endpoint = -1;
    }
    chart->mark_dirty();
}

// ---- Liquidity bands (order-book depth overlay) ---------------------------

extern "C" void vroom_chart_set_liquidity(VroomChart* chart,
                                          const VroomBand* bands, size_t count,
                                          const VroomLiquidityStyle* style) {
    if (!chart) return;
    chart->bands.assign(bands, bands + count);
    if (style) chart->liquidity_style = *style;
    chart->mark_dirty();
}

// ---- Price status lines ---------------------------------------------------

extern "C" void vroom_chart_set_price_lines(VroomChart* chart,
                                            const VroomPriceLine* lines,
                                            size_t count,
                                            const VroomPriceLineStyle* style) {
    if (!chart) return;
    // Deep-copy into the owning form: the label strings live in the chart, so the
    // caller's buffers may be freed as soon as this returns.
    chart->price_lines.clear();
    chart->price_lines.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const VroomPriceLine& src = lines[i];
        VroomChart::StoredPriceLine pl;
        pl.price = src.price;
        pl.color = src.color;
        pl.width = src.width;
        pl.line_style = src.line_style;
        if (src.text) pl.text = src.text;
        if (src.quantity) pl.quantity = src.quantity;
        pl.flags = src.flags;
        chart->price_lines.push_back(std::move(pl));
    }
    if (style) chart->price_line_style = *style;
    // Drop interaction state that no longer refers to a live line — otherwise a
    // removed line would leave a stuck hover highlight or drag preview.
    const auto n = static_cast<int32_t>(count);
    if (chart->hovered_price_line >= n) {
        chart->hovered_price_line = -1;
        chart->hovered_price_line_part = -1;
    }
    if (chart->dragged_price_line >= n) chart->dragged_price_line = -1;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_price_line_hover(VroomChart* chart, int32_t index,
                                                 int32_t part) {
    if (!chart) return;
    if (index < 0 || index >= static_cast<int32_t>(chart->price_lines.size())) {
        index = -1;
        part = -1;
    }
    if (chart->hovered_price_line == index && chart->hovered_price_line_part == part) {
        return;  // hover fires on every pointer move; don't redraw for nothing
    }
    chart->hovered_price_line = index;
    chart->hovered_price_line_part = part;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_price_line_drag(VroomChart* chart, int32_t index,
                                                double price) {
    if (!chart) return;
    if (index < 0 || index >= static_cast<int32_t>(chart->price_lines.size())) {
        if (chart->dragged_price_line == -1) return;
        chart->dragged_price_line = -1;
        chart->mark_dirty();
        return;
    }
    chart->dragged_price_line = index;
    chart->dragged_price_line_price = price;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_draft(VroomChart* chart, int64_t a_time,
                                      double a_price, bool has_b, int64_t b_time,
                                      double b_price, bool guide, uint32_t color,
                                      float width, int32_t kind) {
    if (!chart) return;
    chart->draft_active = true;
    chart->draft_a = VroomDrawPoint{a_time, a_price};
    chart->draft_has_b = has_b;
    chart->draft_b = VroomDrawPoint{b_time, b_price};
    chart->draft_guide = guide;
    chart->draft_color = color;
    chart->draft_width = width;
    chart->draft_kind = kind;
    chart->mark_dirty();
}

extern "C" void vroom_chart_start_draft_stroke(VroomChart* chart, uint32_t color,
                                               float width) {
    if (!chart) return;
    chart->draft_active = true;
    chart->draft_kind = 2;
    chart->draft_guide = true;
    chart->draft_has_b = false;
    chart->draft_color = color;
    chart->draft_width = width;
    chart->draft_points.clear();
    chart->mark_dirty();
}

extern "C" void vroom_chart_append_draft_point(VroomChart* chart, int64_t time_ms,
                                               double price) {
    if (!chart) return;
    if (!chart->draft_active || chart->draft_kind != 2) return;
    chart->draft_points.push_back(VroomDrawPoint{time_ms, price});
    // draft_a doubles as the stroke's first point for the node-dot code path.
    if (chart->draft_points.size() == 1) chart->draft_a = chart->draft_points.front();
    chart->mark_dirty();
}

extern "C" void vroom_chart_clear_draft(VroomChart* chart) {
    if (!chart) return;
    if (!chart->draft_active) return;
    chart->draft_active = false;
    chart->draft_points.clear();
    chart->mark_dirty();
}

extern "C" bool vroom_chart_coord_at(VroomChart* chart, float x_px, float y_px,
                                     VroomCoord* out) {
    if (!chart || !out || chart->candles.empty()) return false;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (window_ms <= 0) return false;
    const auto lay = chart->layout();
    // Mirror draw_chart's bounds: the frozen manual scale, else the auto-fit
    // over the visible slice — so y_to_price matches what's on screen.
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(chart->candles.data() + range.start, n);
    out->time_ms =
        vroom::time_at_x(lay, chart->visible_start_ms, window_ms, x_px);
    out->price = vroom::y_to_price(lay, bounds, y_px);
    return true;
}

extern "C" bool vroom_chart_project(VroomChart* chart, int64_t time_ms,
                                    double price, float* out_x, float* out_y) {
    if (!chart || chart->candles.empty()) return false;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (window_ms <= 0) return false;
    const auto lay = chart->layout();
    // Same bounds as coord_at / draw_chart so the projection matches the render.
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(chart->candles.data() + range.start, n);
    if (out_x)
        *out_x = vroom::x_at_time(lay, chart->visible_start_ms, window_ms, time_ms);
    if (out_y) *out_y = vroom::price_to_y(lay, bounds, price);
    return true;
}

extern "C" bool vroom_chart_hit_test_drawing(VroomChart* chart, float x_px,
                                             float y_px, int32_t* out_index,
                                             int32_t* out_part, float* out_t) {
    if (!chart || chart->candles.empty()) return false;
    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    if (window_ms <= 0) return false;
    const auto lay = chart->layout();
    // Same bounds as coord_at / draw_chart so screen geometry matches the render.
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(chart->candles.data() + range.start, n);
    const auto hit = vroom::drawings::hit_test(*chart, lay, bounds, window_ms, x_px, y_px);
    if (hit.index < 0) return false;
    if (out_index) *out_index = hit.index;
    if (out_part) *out_part = hit.part;
    if (out_t) *out_t = hit.t;
    return true;
}

extern "C" bool vroom_chart_hit_test_price_line(VroomChart* chart, float x_px,
                                                float y_px, int32_t* out_index,
                                                int32_t* out_part) {
    if (!chart || chart->price_lines.empty() || chart->candles.empty()) return false;
    const auto lay = chart->layout();
    // Same bounds and pane geometry as draw_chart, so the grab regions line up
    // with what's on screen.
    const auto range = vroom::visible_indices(
        chart->candles.data(), chart->candles.size(),
        chart->visible_start_ms, chart->visible_end_ms);
    const size_t n = range.end - range.start;
    const auto bounds =
        chart->price_bounds_manual
            ? chart->price_bounds
            : vroom::auto_price_bounds(chart->candles.data() + range.start, n);
    const float candle_area_h = vroom::price_pane_bottom(lay);
    const float candle_right =
        chart->width_px - lay.y_axis_width_px - lay.right_padding_px;
    const auto hit = vroom::price_lines::hit_test(*chart, lay, bounds, candle_right,
                                                  candle_area_h, x_px, y_px);
    if (hit.index < 0) return false;
    if (out_index) *out_index = hit.index;
    if (out_part) *out_part = hit.part;
    return true;
}

extern "C" void vroom_chart_set_selected_drawing(VroomChart* chart,
                                                 int32_t index,
                                                 int32_t grabbed_endpoint) {
    if (!chart) return;
    if (index < 0 || index >= static_cast<int32_t>(chart->drawings.size())) {
        index = -1;
        grabbed_endpoint = -1;
    }
    chart->selected_drawing = index;
    chart->grabbed_endpoint = grabbed_endpoint;
    chart->mark_dirty();
}

extern "C" void vroom_chart_move_drawing_endpoint(VroomChart* chart, int32_t index,
                                                  int32_t endpoint, int64_t time_ms,
                                                  double price) {
    if (!chart) return;
    if (index < 0 || index >= static_cast<int32_t>(chart->drawings.size())) return;
    VroomDrawPoint& pt = endpoint == 0 ? chart->drawings[index].a
                                       : chart->drawings[index].b;
    pt.time_ms = time_ms;
    pt.price = price;
    chart->mark_dirty();
}

extern "C" void vroom_chart_translate_drawing(VroomChart* chart, int32_t index,
                                              int64_t d_time_ms, double d_price) {
    if (!chart) return;
    if (index < 0 || index >= static_cast<int32_t>(chart->drawings.size())) return;
    auto& d = chart->drawings[index];
    const auto shift = [&](VroomDrawPoint& p) {
        p.time_ms += d_time_ms;
        p.price += d_price;
    };
    shift(d.a);
    shift(d.b);
    for (VroomDrawPoint& p : d.points) shift(p);
    chart->mark_dirty();
}

// ---- Indicators -----------------------------------------------------------

extern "C" void vroom_chart_set_rsi(VroomChart* chart, const VroomRSI* cfg) {
    if (!chart || !cfg) return;
    VroomRSI next = *cfg;
    next.enabled = next.enabled ? 1 : 0;
    next.ma_visible = next.ma_visible ? 1 : 0;
    next.line_visible = next.line_visible ? 1 : 0;
    next.bands_visible = next.bands_visible ? 1 : 0;
    if (next.period < 2) next.period = 2;
    if (next.ma_period < 1) next.ma_period = 1;
    next.upper_band = std::clamp(next.upper_band, 0.0, 100.0);
    next.lower_band = std::clamp(next.lower_band, 0.0, 100.0);

    // Only the series-affecting fields force a recompute; band levels, color,
    // width, and visibility changes are render-only. The trendline is one of
    // them: it isn't computed while hidden, so revealing it needs a recompute.
    const VroomRSI& cur = chart->rsi;
    const bool recompute = cur.enabled != next.enabled ||
                           cur.period != next.period ||
                           cur.ma_visible != next.ma_visible ||
                           cur.ma_period != next.ma_period ||
                           cur.ma_kind != next.ma_kind;

    // Pane order: claim the next slot on an off->on transition, release on off.
    if (next.enabled && !cur.enabled) chart->rsi_order = chart->pane_seq++;
    else if (!next.enabled) chart->rsi_order = -1;

    chart->rsi = next;
    if (recompute) chart->rsi_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_macd(VroomChart* chart, const VroomMACD* cfg) {
    if (!chart || !cfg) return;
    VroomMACD next = *cfg;
    next.enabled = next.enabled ? 1 : 0;
    next.line_visible = next.line_visible ? 1 : 0;
    next.signal_visible = next.signal_visible ? 1 : 0;
    next.hist_visible = next.hist_visible ? 1 : 0;
    next.zero_visible = next.zero_visible ? 1 : 0;
    if (next.fast < 1) next.fast = 1;
    if (next.slow < 1) next.slow = 1;
    if (next.slow <= next.fast) next.slow = next.fast + 1;
    if (next.signal < 1) next.signal = 1;

    // Only the series-affecting fields force a recompute; color, width, and
    // visibility changes are render-only.
    const VroomMACD& cur = chart->macd;
    const bool recompute = cur.enabled != next.enabled ||
                           cur.fast != next.fast ||
                           cur.slow != next.slow ||
                           cur.signal != next.signal ||
                           cur.source != next.source ||
                           cur.ma_kind != next.ma_kind ||
                           cur.signal_ma_kind != next.signal_ma_kind;

    if (next.enabled && !cur.enabled) chart->macd_order = chart->pane_seq++;
    else if (!next.enabled) chart->macd_order = -1;

    chart->macd = next;
    if (recompute) chart->macd_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_overlays(VroomChart* chart,
                                         const VroomOverlay* overlays,
                                         size_t count) {
    if (!chart) return;
    chart->overlays.assign(overlays, overlays + count);
    chart->overlays_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_vwap(VroomChart* chart, const VroomVWAP* cfg) {
    if (!chart || !cfg) return;
    VroomVWAP next = *cfg;
    next.enabled = next.enabled ? 1 : 0;
    // Keep the offset within a day [0, 1440).
    next.reset_offset_min %= 1440;
    if (next.reset_offset_min < 0) next.reset_offset_min += 1440;

    // Only the series-affecting fields force a recompute; color and width
    // changes are render-only.
    const VroomVWAP& cur = chart->vwap;
    const bool recompute = cur.enabled != next.enabled ||
                           cur.reset_offset_min != next.reset_offset_min;

    chart->vwap = next;
    if (recompute) chart->vwap_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_bollinger(VroomChart* chart,
                                          const VroomBollinger* cfg) {
    if (!chart || !cfg) return;
    VroomBollinger next = *cfg;
    next.enabled = next.enabled ? 1 : 0;
    next.fill_enabled = next.fill_enabled ? 1 : 0;
    if (next.period < 1) next.period = 1;
    if (!(next.mult >= 0.f)) next.mult = 0.f;
    next.fill_opacity = std::clamp(next.fill_opacity, 0.f, 1.f);

    // Only the series-affecting fields force a recompute; style and fill
    // changes are render-only.
    const VroomBollinger& cur = chart->bollinger;
    const bool recompute = cur.enabled != next.enabled ||
                           cur.period != next.period ||
                           cur.mult != next.mult ||
                           cur.source != next.source ||
                           cur.basis_kind != next.basis_kind;
    chart->bollinger = next;
    if (recompute) chart->bollinger_dirty = true;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_volume(VroomChart* chart,
                                       const VroomVolume* cfg) {
    if (!chart || !cfg) return;
    VroomVolume next = *cfg;
    next.enabled = next.enabled ? 1 : 0;
    // Negative means "inherit", so only clamp the ranges once a value is set.
    if (next.height_frac >= 0.f) next.height_frac = std::min(next.height_frac, 1.f);
    if (next.opacity >= 0.f) next.opacity = std::min(next.opacity, 1.f);
    chart->volume = next;
    // Snap the collapse to the new target, the way set_chart_type snaps the
    // candle↔line morph. A host that wants the toggle animated overrides this
    // from its frame loop before the next paint.
    chart->volume_collapse_t = next.enabled ? 0.f : 1.f;
    chart->mark_dirty();
}

extern "C" void vroom_chart_set_volume_collapse(VroomChart* chart, float t,
                                               int32_t easing) {
    if (!chart) return;
    chart->volume_collapse_t = std::clamp(t, 0.f, 1.f);
    chart->volume_collapse_easing = easing;
    chart->mark_dirty();
}

// ---- Direct draw (used by hosts that don't need the SkPicture cache) ------

extern "C" void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas) {
    if (!chart || !canvas) return;
    chart->draw_chart(canvas);
}

extern "C" bool vroom_chart_is_animating(VroomChart* chart) {
    return chart ? chart->is_animating_now() : false;
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
