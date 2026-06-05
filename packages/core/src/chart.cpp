// VroomChart core — Phase 1.
//
// Owns the candle buffer, layout, theme, and a cached sk_sp<SkPicture> built
// by SkPictureRecorder. The chart picture is rebuilt only when input state
// dirties (new candles, new size). The JSI bridge calls `render_picture()` to
// get the latest picture and wraps it as a JsiSkPicture for JS.

#include "vroom/vroom_chart.h"

#include <cstdlib>
#include <utility>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "chart_internal.h"
#include "viewport.h"

#include <algorithm>

namespace vroom {

// Tick algorithms — pick a label interval based on current scale so labels
// land on canonical boundaries with reasonable spacing. As the user pans or
// zooms, labels translate naturally (they're tied to time/price values);
// the interval only swaps when spacing crosses a threshold.

// Minimum spacing between adjacent x-axis labels in pixels. Smaller → more
// labels visible, finer interval; larger → sparser labels.
static constexpr float kXLabelMinSpacing = 80.f;
// Target spacing for y-axis labels — used to derive a target count.
static constexpr float kYLabelTargetSpacing = 55.f;

// Smallest time interval whose pixel-spacing meets the minimum. Returns ms.
static int64_t pick_time_interval(int64_t window_ms, float candle_area_w) {
    static constexpr int64_t kIntervals[] = {
        60'000LL,                // 1m
        5 * 60'000LL,            // 5m
        15 * 60'000LL,           // 15m
        30 * 60'000LL,           // 30m
        60 * 60'000LL,           // 1h
        2 * 60 * 60'000LL,       // 2h
        4 * 60 * 60'000LL,       // 4h
        6 * 60 * 60'000LL,       // 6h
        12 * 60 * 60'000LL,      // 12h
        24 * 60 * 60'000LL,      // 1d
        2 * 24 * 60 * 60'000LL,  // 2d
        7 * 24 * 60 * 60'000LL,  // 1w
    };
    if (window_ms <= 0 || candle_area_w <= 0.f) return kIntervals[0];
    const double ms_per_px =
        static_cast<double>(window_ms) / candle_area_w;
    for (int64_t i : kIntervals) {
        if (static_cast<double>(i) / ms_per_px >= kXLabelMinSpacing) {
            return i;
        }
    }
    return kIntervals[sizeof(kIntervals) / sizeof(kIntervals[0]) - 1];
}

// "Nice number" tick selection: snaps to 1, 2, or 5 × 10ⁿ.
static double pick_price_interval(double range, float candle_area_h) {
    if (range <= 0.0 || candle_area_h <= 0.f) return 1.0;
    const int target_count =
        std::max(2, static_cast<int>(candle_area_h / kYLabelTargetSpacing));
    const double rough = range / target_count;
    const double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
    const double normalized = rough / magnitude;
    double nice;
    if (normalized < 1.5) nice = 1.0;
    else if (normalized < 3.5) nice = 2.0;
    else if (normalized < 7.5) nice = 5.0;
    else nice = 10.0;
    return nice * magnitude;
}

// Global typeface for axis labels. Set once at startup by the bridge layer
// (which loads it via RN-Skia's platform-context font manager). All charts
// share it — no per-chart font is supported yet.
static sk_sp<SkTypeface> g_axis_typeface;

void set_axis_typeface(sk_sp<SkTypeface> typeface) {
    g_axis_typeface = std::move(typeface);
    // Existing charts will pick up the new typeface on next draw, but their
    // axis_width_px was computed without a typeface so it'll still show the
    // ratio fallback until set_candles is called again. Acceptable for now —
    // the typeface is normally set before the first chart is created.
}

namespace {

// Default theme — match the test-bench dark background.
constexpr uint32_t kDefaultColors[VROOM_COLOR_COUNT_] = {
    0xff0d1117,  // BACKGROUND
    0xff26a69a,  // BULL
    0xffef5350,  // BEAR
    0xffaaaaaa,  // WICK
    0xff21262d,  // GRID
    0xffc9d1d9,  // AXIS_TEXT
    0xffc9d1d9,  // CROSSHAIR
    0xff161b22,  // TOOLTIP_BG
    0xffc9d1d9,  // TOOLTIP_TEXT
};

constexpr float kDefaultFloats[VROOM_FLOAT_COUNT_] = {
    0.55f,  // CANDLE_WIDTH_RATIO — body fills 55% of slot, 45% gap
    1.0f,   // WICK_WIDTH_PX
    6.f,    // RIGHT_PADDING_PX — small gutter between candles and y-axis
    11.f,   // AXIS_FONT_SIZE_PX
    0.18f,  // Y_AXIS_WIDTH_RATIO — fallback before font is loaded
    22.f,   // X_AXIS_HEIGHT_PX — bottom strip for time labels
};

}  // namespace
}  // namespace vroom

struct VroomChart {
    VroomCallbacks cb{};
    void* user_ctx = nullptr;

    std::vector<VroomCandle> candles;

    float width_px = 0.f;
    float height_px = 0.f;
    float px_ratio = 1.f;

    // 0/0 = show everything (Phase 1 behavior). Set via set_visible_range.
    int64_t visible_start_ms = 0;
    int64_t visible_end_ms = 0;

    // Inferred from the gap between consecutive candles when data is loaded.
    // Used so each candle occupies a stable pixel slot regardless of how
    // many fall in the visible window (or whether the window extends past
    // the end of the data into "future" empty space).
    int64_t candle_duration_ms = 60'000;

    bool   crosshair_active = false;
    float  crosshair_x_px = 0.f;
    float  crosshair_y_px = 0.f;

    uint32_t colors[VROOM_COLOR_COUNT_]{};
    float    floats[VROOM_FLOAT_COUNT_]{};

    sk_sp<SkPicture> chart_picture;
    bool chart_dirty = true;

    // Cached axis width in pixels — sized to fit the widest price label
    // (formatted "%.2f" of the dataset's max high). Recomputed when candles,
    // size, or font settings change. 0 means "not yet computed; use ratio
    // fallback in layout()".
    float axis_width_px = 0.f;

    // Axis-label fade animation. Each label key (a y-axis price or x-axis
    // time-ms) carries a current opacity and a target. When the active set
    // changes (e.g., interval swap on zoom), labels not in the new set get
    // target=0 and fade out; new labels get target=1 and fade in. Opacities
    // advance toward targets by ~kFadeRate per second; render_chart_picture
    // keeps rebuilding until everything settles.
    struct YLabelFade {
        double price;
        float opacity = 0.f;
        float target = 1.f;
    };
    struct XLabelFade {
        int64_t time_ms;
        float opacity = 0.f;
        float target = 1.f;
    };
    std::vector<YLabelFade> y_fades;
    std::vector<XLabelFade> x_fades;
    std::chrono::steady_clock::time_point last_anim_tick{};
    bool anim_started = false;

    // Price (y-axis) bounds are STATE, not derived per-frame from the
    // visible window. Set on first setCandles from the initial visible
    // window, then mutated only by pinch zoom. Panning preserves them so
    // the price scale stays steady while scrolling horizontally.
    vroom::PriceBounds price_bounds{0.0, 1.0};
    bool price_bounds_initialized = false;

    VroomChart() {
        for (int i = 0; i < VROOM_COLOR_COUNT_; ++i) colors[i] = vroom::kDefaultColors[i];
        for (int i = 0; i < VROOM_FLOAT_COUNT_; ++i) floats[i] = vroom::kDefaultFloats[i];
    }

    void mark_dirty() {
        chart_dirty = true;
        if (cb.on_redraw_requested) cb.on_redraw_requested(user_ctx);
    }

    vroom::Layout layout() const {
        const float axis_w = axis_width_px > 0.f
            ? axis_width_px
            : width_px * floats[VROOM_FLOAT_Y_AXIS_WIDTH_RATIO];
        return vroom::Layout{
            width_px,
            height_px,
            axis_w,
            floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX],
            floats[VROOM_FLOAT_RIGHT_PADDING_PX],
            floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO],
            0.05f,
            0.05f,
        };
    }

    // Recomputes axis_width_px from the formatted price-bound label widths.
    // Uses the wider of price_bounds.max / price_bounds.min so labels fit
    // regardless of negatives or differing digit counts. No-ops gracefully
    // if the typeface isn't loaded yet — layout() falls back to the ratio.
    void recompute_axis_width() {
        if (!vroom::g_axis_typeface) {
            axis_width_px = 0.f;
            return;
        }
        double hi, lo;
        if (price_bounds_initialized) {
            hi = price_bounds.max;
            lo = price_bounds.min;
        } else if (!candles.empty()) {
            hi = candles.front().high;
            lo = candles.front().low;
            for (const auto& c : candles) {
                if (c.high > hi) hi = c.high;
                if (c.low < lo) lo = c.low;
            }
        } else {
            axis_width_px = 0.f;
            return;
        }

        SkFont font(vroom::g_axis_typeface,
                    floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        char buf_hi[32], buf_lo[32];
        std::snprintf(buf_hi, sizeof(buf_hi), "%.2f", hi);
        std::snprintf(buf_lo, sizeof(buf_lo), "%.2f", lo);
        const float w_hi = font.measureText(
            buf_hi, std::strlen(buf_hi), SkTextEncoding::kUTF8);
        const float w_lo = font.measureText(
            buf_lo, std::strlen(buf_lo), SkTextEncoding::kUTF8);
        const float text_w = std::max(w_hi, w_lo);

        constexpr float kPaddingLeft = 8.f;   // separator ↔ text
        constexpr float kPaddingRight = 6.f;  // text ↔ screen edge
        axis_width_px = text_w + kPaddingLeft + kPaddingRight;
    }

    void draw_chart(SkCanvas* canvas) {
        const auto lay = layout();

        // 1. Background
        SkPaint bg;
        bg.setColor(colors[VROOM_COLOR_BACKGROUND]);
        canvas->drawRect(SkRect::MakeWH(width_px, height_px), bg);

        if (candles.empty()) return;

        // 2. Visible-range + bounds + layout geometry
        const auto range = vroom::visible_indices(
            candles.data(), candles.size(),
            visible_start_ms, visible_end_ms);
        const size_t n = range.end - range.start;
        if (n == 0) return;
        const ::VroomCandle* visible = candles.data() + range.start;

        const auto bounds = price_bounds_initialized
            ? price_bounds
            : vroom::price_bounds(visible, n);
        const int64_t window_ms = visible_end_ms - visible_start_ms;
        const float body_w = vroom::candle_body_width(
            lay, window_ms, candle_duration_ms);

        const float candle_area_h = height_px - lay.x_axis_height_px;
        const float candle_right =
            width_px - lay.y_axis_width_px - lay.right_padding_px;
        const float candle_area_w = candle_right;

        // 3. Update label fade state ONCE per frame — both gridlines and
        // labels share these opacities so their animations stay in lockstep.
        // Consumes last_dt_seconds.
        update_y_label_fades(lay, bounds);
        update_x_label_fades(lay);

        // 4. Gridlines — drawn before candles so candle bodies overlay them.
        draw_y_gridlines(canvas, lay, bounds, candle_right, candle_area_h);
        draw_x_gridlines(canvas, candle_area_w, candle_area_h);

        // 5. Candles (wicks + bodies)
        SkPaint bull_paint;
        bull_paint.setAntiAlias(true);
        bull_paint.setColor(colors[VROOM_COLOR_BULL]);

        SkPaint bear_paint;
        bear_paint.setAntiAlias(true);
        bear_paint.setColor(colors[VROOM_COLOR_BEAR]);

        SkPaint wick_bull;
        wick_bull.setAntiAlias(true);
        wick_bull.setColor(colors[VROOM_COLOR_BULL]);
        wick_bull.setStrokeWidth(floats[VROOM_FLOAT_WICK_WIDTH_PX]);
        wick_bull.setStyle(SkPaint::kStroke_Style);

        SkPaint wick_bear = wick_bull;
        wick_bear.setColor(colors[VROOM_COLOR_BEAR]);

        const float half_body = body_w * 0.5f;

        for (size_t i = 0; i < n; ++i) {
            const auto& c = visible[i];
            const bool bull = c.close >= c.open;

            const float cx = vroom::candle_center_x(
                lay, c.time_ms, candle_duration_ms,
                visible_start_ms, window_ms);
            const float y_high = vroom::price_to_y(lay, bounds, c.high);
            const float y_low = vroom::price_to_y(lay, bounds, c.low);
            const float y_open = vroom::price_to_y(lay, bounds, c.open);
            const float y_close = vroom::price_to_y(lay, bounds, c.close);

            canvas->drawLine(cx, y_high, cx, y_low,
                             bull ? wick_bull : wick_bear);

            const float y_top = std::min(y_open, y_close);
            const float y_bot = std::max(y_open, y_close);
            const float h = std::max(1.f, y_bot - y_top);
            canvas->drawRect(
                SkRect::MakeXYWH(cx - half_body, y_top, body_w, h),
                bull ? bull_paint : bear_paint);
        }

        // 6. Axis backgrounds (mask any candle overflow) + separator lines
        SkPaint axis_bg;
        axis_bg.setColor(colors[VROOM_COLOR_BACKGROUND]);
        canvas->drawRect(
            SkRect::MakeXYWH(0, candle_area_h, candle_right,
                             lay.x_axis_height_px),
            axis_bg);
        const float axis_block_w = width_px - candle_right;
        canvas->drawRect(
            SkRect::MakeXYWH(candle_right, 0, axis_block_w, height_px),
            axis_bg);

        SkPaint border;
        border.setColor(colors[VROOM_COLOR_GRID]);
        border.setStrokeWidth(1.f);
        border.setAntiAlias(true);
        canvas->drawLine(candle_right, 0, candle_right, candle_area_h, border);
        canvas->drawLine(0, candle_area_h, candle_right, candle_area_h, border);

        // 7. Labels (read from y_fades / x_fades, no state mutation here)
        draw_y_labels(canvas, lay, bounds);
        draw_x_labels(canvas, lay);

        // 8. GC fades that have fully faded out and aren't coming back.
        gc_y_label_fades();
        gc_x_label_fades();
    }

    // Match-tolerance for fade-keys: a label price like 65.000000001 should
    // still match an existing fade at 65.0. We compare with a tolerance
    // proportional to the magnitude.
    static bool price_matches(double a, double b) {
        const double tol = std::max(std::abs(a), std::abs(b)) * 1e-9 + 1e-9;
        return std::abs(a - b) < tol;
    }

    // ----- Y-axis fade update / draw / GC --------------------------------
    //
    // Split out so gridlines and labels share the SAME `y_fades` opacities,
    // and so dt is consumed exactly once per frame (in update_y_label_fades).

    // Steps 1-3 from the old draw_axis_labels: set fade targets, walk the
    // active label set, advance opacities by dt.
    void update_y_label_fades(const vroom::Layout& lay,
                              const vroom::PriceBounds& bounds) {
        const double range = bounds.max - bounds.min;
        if (range <= 0.0) return;

        const float candle_area_h = height_px - lay.x_axis_height_px;
        const double interval = vroom::pick_price_interval(range, candle_area_h);
        if (interval <= 0.0) return;

        for (auto& f : y_fades) f.target = 0.f;

        const double first = std::ceil(bounds.min / interval) * interval;
        constexpr int kMaxLabels = 64;
        int promoted = 0;
        for (double price = first;
             price <= bounds.max && promoted < kMaxLabels;
             price += interval, ++promoted) {
            bool found = false;
            for (auto& f : y_fades) {
                if (price_matches(f.price, price)) {
                    f.target = 1.f;
                    found = true;
                    break;
                }
            }
            if (!found) {
                y_fades.push_back(YLabelFade{price, 0.f, 1.f});
            }
        }

        const float step = kFadeRate * last_dt_seconds;
        for (auto& f : y_fades) {
            if (last_dt_seconds == 0.f) {
                // First render (or resume after a long idle that capped dt
                // to 0): snap to target so labels/gridlines are visible
                // immediately rather than waiting on a fade tick — the JS
                // anim loop only kicks in after a gesture.
                f.opacity = f.target;
            } else if (f.opacity < f.target) {
                f.opacity = std::min(f.target, f.opacity + step);
            } else if (f.opacity > f.target) {
                f.opacity = std::max(f.target, f.opacity - step);
            }
        }
    }

    // Horizontal gridlines aligned with each y-label row, sharing label
    // opacity so they fade in/out together when the price interval swaps.
    void draw_y_gridlines(SkCanvas* canvas,
                          const vroom::Layout& lay,
                          const vroom::PriceBounds& bounds,
                          float candle_right,
                          float candle_area_h) const {
        SkPaint grid;
        grid.setColor(colors[VROOM_COLOR_GRID]);
        grid.setStrokeWidth(1.f);
        grid.setAntiAlias(true);
        for (const auto& f : y_fades) {
            if (f.opacity <= 1e-3f) continue;
            const float y = vroom::price_to_y(lay, bounds, f.price);
            if (y < 0.f || y > candle_area_h) continue;
            grid.setAlphaf(f.opacity);
            canvas->drawLine(0.f, y, candle_right, y, grid);
        }
    }

    // Step 4 from the old function — read-only render pass over y_fades.
    void draw_y_labels(SkCanvas* canvas,
                       const vroom::Layout& lay,
                       const vroom::PriceBounds& bounds) const {
        if (!vroom::g_axis_typeface) return;

        const float candle_area_h = height_px - lay.x_axis_height_px;

        SkFont font(vroom::g_axis_typeface,
                    floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        const float cap_h = metrics.fCapHeight > 0 ? metrics.fCapHeight
                                                   : -metrics.fAscent * 0.7f;

        SkPaint text_paint;
        text_paint.setColor(colors[VROOM_COLOR_AXIS_TEXT]);
        text_paint.setAntiAlias(true);

        constexpr float kRightInset = 6.f;

        for (const auto& f : y_fades) {
            if (f.opacity <= 1e-3f) continue;
            const float y = vroom::price_to_y(lay, bounds, f.price);

            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.2f", f.price);
            const size_t len = std::strlen(buf);
            const float text_w = font.measureText(
                buf, len, SkTextEncoding::kUTF8);

            const float text_x = lay.width_px - kRightInset - text_w;
            const float baseline_y = y + cap_h * 0.5f;
            if (baseline_y - cap_h < 0.f) continue;
            if (baseline_y > candle_area_h) continue;

            text_paint.setAlphaf(f.opacity);
            canvas->drawString(buf, text_x, baseline_y, font, text_paint);
        }
    }

    // Step 5 — erase entries that have settled at fully-faded-out.
    void gc_y_label_fades() {
        y_fades.erase(
            std::remove_if(y_fades.begin(), y_fades.end(),
                           [](const YLabelFade& f) {
                               return f.opacity <= 0.f && f.target <= 0.f;
                           }),
            y_fades.end());
    }

    // ----- X-axis fade update / draw / GC --------------------------------

    void update_x_label_fades(const vroom::Layout& lay) {
        if (candles.empty()) return;
        if (visible_end_ms <= visible_start_ms) return;

        const float candle_area_w =
            lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
        if (candle_area_w <= 0.f) return;

        const int64_t window_ms = visible_end_ms - visible_start_ms;
        const int64_t interval =
            vroom::pick_time_interval(window_ms, candle_area_w);
        if (interval <= 0) return;

        for (auto& f : x_fades) f.target = 0.f;

        int64_t t = (visible_start_ms / interval) * interval;
        if (t < visible_start_ms) t += interval;
        constexpr int kMaxLabels = 64;
        int promoted = 0;
        for (; t <= visible_end_ms && promoted < kMaxLabels;
             t += interval, ++promoted) {
            bool found = false;
            for (auto& f : x_fades) {
                if (f.time_ms == t) {
                    f.target = 1.f;
                    found = true;
                    break;
                }
            }
            if (!found) {
                x_fades.push_back(XLabelFade{t, 0.f, 1.f});
            }
        }

        const float step = kFadeRate * last_dt_seconds;
        for (auto& f : x_fades) {
            if (last_dt_seconds == 0.f) {
                f.opacity = f.target;  // snap on first render — see y twin
            } else if (f.opacity < f.target) {
                f.opacity = std::min(f.target, f.opacity + step);
            } else if (f.opacity > f.target) {
                f.opacity = std::max(f.target, f.opacity - step);
            }
        }
    }

    // Vertical gridlines aligned with each x-label column.
    void draw_x_gridlines(SkCanvas* canvas,
                          float candle_area_w,
                          float candle_area_h) const {
        if (visible_end_ms <= visible_start_ms) return;
        const int64_t window_ms = visible_end_ms - visible_start_ms;
        if (window_ms <= 0 || candle_area_w <= 0.f) return;

        SkPaint grid;
        grid.setColor(colors[VROOM_COLOR_GRID]);
        grid.setStrokeWidth(1.f);
        grid.setAntiAlias(true);
        for (const auto& f : x_fades) {
            if (f.opacity <= 1e-3f) continue;
            const float frac =
                static_cast<float>(f.time_ms - visible_start_ms) /
                static_cast<float>(window_ms);
            const float x = frac * candle_area_w;
            if (x < 0.f || x > candle_area_w) continue;
            grid.setAlphaf(f.opacity);
            canvas->drawLine(x, 0.f, x, candle_area_h, grid);
        }
    }

    void draw_x_labels(SkCanvas* canvas, const vroom::Layout& lay) const {
        if (!vroom::g_axis_typeface || candles.empty()) return;
        if (visible_end_ms <= visible_start_ms) return;

        const float candle_area_h = height_px - lay.x_axis_height_px;
        const float candle_area_w =
            lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
        if (candle_area_w <= 0.f) return;

        const int64_t window_ms = visible_end_ms - visible_start_ms;
        // Recompute interval here just to decide HH:MM vs MM/DD formatting.
        // Cheap and keeps the function self-contained.
        const int64_t interval =
            vroom::pick_time_interval(window_ms, candle_area_w);

        SkFont font(vroom::g_axis_typeface,
                    floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        const float cap_h = metrics.fCapHeight > 0 ? metrics.fCapHeight
                                                   : -metrics.fAscent * 0.7f;

        SkPaint text_paint;
        text_paint.setColor(colors[VROOM_COLOR_AXIS_TEXT]);
        text_paint.setAntiAlias(true);

        const float baseline_y =
            candle_area_h + (lay.x_axis_height_px + cap_h) * 0.5f;
        const bool use_date = interval >= 24LL * 60 * 60 * 1000;
        for (const auto& f : x_fades) {
            if (f.opacity <= 1e-3f) continue;
            const float frac =
                static_cast<float>(f.time_ms - visible_start_ms) /
                static_cast<float>(window_ms);
            const float x_center = frac * candle_area_w;

            char buf[16];
            const time_t time_s = static_cast<time_t>(f.time_ms / 1000);
            struct tm tm_buf;
            localtime_r(&time_s, &tm_buf);
            if (use_date) {
                std::snprintf(buf, sizeof(buf), "%02d/%02d",
                              tm_buf.tm_mon + 1, tm_buf.tm_mday);
            } else {
                std::snprintf(buf, sizeof(buf), "%02d:%02d",
                              tm_buf.tm_hour, tm_buf.tm_min);
            }

            const size_t len = std::strlen(buf);
            const float text_w = font.measureText(
                buf, len, SkTextEncoding::kUTF8);
            const float text_x = x_center - text_w * 0.5f;
            if (text_x < 0.f) continue;
            if (text_x + text_w > candle_area_w) continue;

            text_paint.setAlphaf(f.opacity);
            canvas->drawString(buf, text_x, baseline_y, font, text_paint);
        }
    }

    void gc_x_label_fades() {
        x_fades.erase(
            std::remove_if(x_fades.begin(), x_fades.end(),
                           [](const XLabelFade& f) {
                               return f.opacity <= 0.f && f.target <= 0.f;
                           }),
            x_fades.end());
    }

    void rebuild_chart_picture() {
        // Compute dt for fade animations. We cap large gaps (resume from
        // background, long idle) so we don't snap animations to completion.
        auto now = std::chrono::steady_clock::now();
        float dt = 0.f;
        if (anim_started) {
            dt = std::chrono::duration<float>(now - last_anim_tick).count();
            if (dt > 0.1f) dt = 0.f;
        }
        last_anim_tick = now;
        anim_started = true;

        last_dt_seconds = dt;

        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(width_px, height_px));
        draw_chart(canvas);
        chart_picture = recorder.finishRecordingAsPicture();
        chart_dirty = false;
    }

    bool is_animating_now() const {
        for (const auto& f : y_fades) {
            if (f.opacity != f.target) return true;
        }
        for (const auto& f : x_fades) {
            if (f.opacity != f.target) return true;
        }
        return false;
    }

   private:
    // dt of the current rebuild — passed implicitly to the label drawers via
    // this field so we don't have to retrofit signatures.
    mutable float last_dt_seconds = 0.f;
    static constexpr float kFadeRate = 5.f;  // 1 / 0.2s
};

// ---- C facade ----

VroomChart* vroom_chart_create(const VroomCallbacks* cb, void* user_ctx) {
    auto* c = new VroomChart();
    if (cb) c->cb = *cb;
    c->user_ctx = user_ctx;
    return c;
}

void vroom_chart_destroy(VroomChart* chart) { delete chart; }

void vroom_chart_set_candles(VroomChart* chart, const VroomCandle* data, size_t count) {
    if (!chart) return;
    chart->candles.assign(data, data + count);
    // Infer the candle period from the first interval. Robust enough for
    // uniform-duration series (the only kind we model today).
    if (chart->candles.size() >= 2) {
        const int64_t d = chart->candles[1].time_ms - chart->candles[0].time_ms;
        if (d > 0) chart->candle_duration_ms = d;
    }
    chart->recompute_axis_width();

    // Default the visible window to the most recent ~60 candles when the
    // consumer hasn't set one. A full-data-range default is wrong for the
    // gesture model: panning would shift a window the same width as the
    // data, and candles would fly off the edge / rescale unpleasantly.
    if (chart->visible_start_ms == 0 && chart->visible_end_ms == 0 &&
        !chart->candles.empty()) {
        constexpr size_t kDefaultVisible = 60;
        const size_t start_idx = chart->candles.size() > kDefaultVisible
            ? chart->candles.size() - kDefaultVisible
            : 0;
        chart->visible_start_ms = chart->candles[start_idx].time_ms;
        chart->visible_end_ms = chart->candles.back().time_ms;
    }

    // Snap the price (y-axis) bounds to whatever's visible right now. After
    // this, only pinch zoom changes them; panning preserves them so the
    // candles drift up/down off-screen as the user scrolls historically.
    if (!chart->candles.empty()) {
        const auto idx = vroom::visible_indices(
            chart->candles.data(), chart->candles.size(),
            chart->visible_start_ms, chart->visible_end_ms);
        if (idx.end > idx.start) {
            chart->price_bounds = vroom::price_bounds(
                chart->candles.data() + idx.start, idx.end - idx.start);
            chart->price_bounds_initialized = true;
        }
    }

    chart->mark_dirty();
}

void vroom_chart_append_candle(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c) return;
    chart->candles.push_back(*c);
    chart->mark_dirty();
}

void vroom_chart_update_last(VroomChart* chart, const VroomCandle* c) {
    if (!chart || !c || chart->candles.empty()) return;
    chart->candles.back() = *c;
    chart->mark_dirty();
}

void vroom_chart_set_size(VroomChart* chart, float w, float h, float ratio) {
    if (!chart) return;
    if (chart->width_px == w && chart->height_px == h && chart->px_ratio == ratio) return;
    chart->width_px = w;
    chart->height_px = h;
    chart->px_ratio = ratio;
    chart->mark_dirty();
}

void vroom_chart_set_visible_range(VroomChart* chart, int64_t start_ms, int64_t end_ms) {
    if (!chart) return;
    if (chart->visible_start_ms == start_ms && chart->visible_end_ms == end_ms) return;
    chart->visible_start_ms = start_ms;
    chart->visible_end_ms = end_ms;
    if (chart->cb.on_viewport_changed) {
        chart->cb.on_viewport_changed(chart->user_ctx, start_ms, end_ms);
    }
    chart->mark_dirty();
}

void vroom_chart_pan(VroomChart* chart, float dx_px, float /*dy_px*/) {
    if (!chart || dx_px == 0.f || chart->candles.empty()) return;

    const int64_t window_ms = chart->visible_end_ms - chart->visible_start_ms;
    const float usable_px =
        chart->width_px - chart->floats[VROOM_FLOAT_RIGHT_PADDING_PX];
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

void vroom_chart_translate(VroomChart* chart, float dx_px, float dy_px) {
    if (!chart || chart->candles.empty()) return;
    bool changed = false;

    // Horizontal: shift the time window. Same math + clamping as pan, but
    // we don't fire the redraw notification mid-call — we batch it below.
    if (dx_px != 0.f) {
        const int64_t window_ms =
            chart->visible_end_ms - chart->visible_start_ms;
        const float usable_px =
            chart->width_px - chart->floats[VROOM_FLOAT_RIGHT_PADDING_PX];
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
    // that's actually used for price → y) so the translation feels 1:1
    // against the visible candles.
    if (dy_px != 0.f && chart->price_bounds_initialized) {
        const float candle_area_h =
            chart->height_px - chart->floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX];
        const double draw_h = static_cast<double>(candle_area_h) * 0.9;
        if (draw_h > 0.0) {
            const double range =
                chart->price_bounds.max - chart->price_bounds.min;
            const double dprice =
                (static_cast<double>(dy_px) / draw_h) * range;
            chart->price_bounds.min += dprice;
            chart->price_bounds.max += dprice;
            chart->recompute_axis_width();
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

// Sensitivity for axis-drag scaling. Smaller = more responsive per pixel.
// 300 means a 300px drag roughly doubles or halves the range.
static constexpr double kAxisDragSensitivity = 300.0;

// Min/max candle body width in pixels. The x-axis-drag clamp computes the
// time window that would produce each of these widths given the current
// candle area, and bounds new_window_ms to keep candles visually reasonable.
// Starting values — dial in based on feel.
static constexpr double kMinCandleBodyPx = 1.5;
static constexpr double kMaxCandleBodyPx = 32.0;

void vroom_chart_scale_price_axis(VroomChart* chart, float dy_px) {
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
    chart->recompute_axis_width();

    chart->mark_dirty();
}

void vroom_chart_scale_time_axis(VroomChart* chart, float dx_px) {
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
    const double ratio = chart->floats[VROOM_FLOAT_CANDLE_WIDTH_RATIO];
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

void vroom_chart_get_axis_metrics(VroomChart* chart,
                                   float* out_y_axis_width_px,
                                   float* out_x_axis_height_px) {
    if (!chart) return;
    const auto lay = chart->layout();
    if (out_y_axis_width_px) *out_y_axis_width_px = lay.y_axis_width_px;
    if (out_x_axis_height_px) *out_x_axis_height_px = lay.x_axis_height_px;
}

void vroom_chart_zoom(VroomChart* chart, float scale, float /*fx*/, float fy) {
    if (!chart || scale <= 0.f || scale == 1.f) return;
    if (!chart->price_bounds_initialized) return;

    const float candle_area_h =
        chart->height_px - chart->floats[VROOM_FLOAT_X_AXIS_HEIGHT_PX];
    if (candle_area_h <= 0.f) return;

    const double range = chart->price_bounds.max - chart->price_bounds.min;
    if (range <= 0.0) return;

    // Focal y → price under the current price scale. We keep this price
    // anchored at `fy` while resizing the price range around it. scale > 1
    // (pinch out vertically) narrows the range → candles taller.
    const float frac = std::clamp(fy / candle_area_h, 0.f, 1.f);
    const double focal_price = chart->price_bounds.max - frac * range;

    const double new_range = range / static_cast<double>(scale);
    chart->price_bounds.max = focal_price + frac * new_range;
    chart->price_bounds.min = chart->price_bounds.max - new_range;
    chart->recompute_axis_width();

    chart->mark_dirty();
}

void vroom_chart_set_crosshair(VroomChart* chart, float x, float y) {
    if (!chart) return;
    chart->crosshair_active = true;
    chart->crosshair_x_px = x;
    chart->crosshair_y_px = y;
    chart->mark_dirty();
}

void vroom_chart_clear_crosshair(VroomChart* chart) {
    if (!chart) return;
    chart->crosshair_active = false;
    chart->mark_dirty();
}

void vroom_chart_draw(VroomChart* chart, SkCanvas* canvas) {
    if (!chart || !canvas) return;
    chart->draw_chart(canvas);
}

void vroom_chart_set_color(VroomChart* chart, VroomColorKey key, uint32_t argb) {
    if (!chart || key < 0 || key >= VROOM_COLOR_COUNT_) return;
    if (chart->colors[key] == argb) return;
    chart->colors[key] = argb;
    chart->mark_dirty();
}

void vroom_chart_set_float(VroomChart* chart, VroomFloatKey key, float value) {
    if (!chart || key < 0 || key >= VROOM_FLOAT_COUNT_) return;
    if (chart->floats[key] == value) return;
    chart->floats[key] = value;
    if (key == VROOM_FLOAT_AXIS_FONT_SIZE_PX) {
        chart->recompute_axis_width();
    }
    chart->mark_dirty();
}

namespace vroom {

// Declared in chart_internal.h; implemented here so it has direct access to
// VroomChart's private state.
sk_sp<SkPicture> render_chart_picture(VroomChart* chart) {
    if (!chart) return nullptr;
    if (chart->chart_dirty || !chart->chart_picture || chart->is_animating_now()) {
        chart->rebuild_chart_picture();
    }
    return chart->chart_picture;
}

bool is_animating(VroomChart* chart) {
    return chart ? chart->is_animating_now() : false;
}

}  // namespace vroom
