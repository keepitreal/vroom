#include "labels.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypeface.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "chart.h"
#include "fonts.h"
#include "ticks.h"
#include "viewport.h"

namespace vroom::labels {

namespace {

// Fuzzy equality for price keys: a label like 65.000000001 should still match
// an existing fade entry at 65.0 after a few FP roundtrips.
bool price_matches(double a, double b) {
    const double tol = std::max(std::abs(a), std::abs(b)) * 1e-9 + 1e-9;
    return std::abs(a - b) < tol;
}

// Advance a label's opacity toward its target by `step`. On the first render
// of a chart `chart.last_dt_seconds == 0` (and we'd be stuck at opacity 0
// forever), so we snap to the target then; subsequent frames fade normally.
template <typename Fade>
void advance(Fade& f, float step, float dt) {
    if (dt == 0.f) {
        f.opacity = f.target;
    } else if (f.opacity < f.target) {
        f.opacity = std::min(f.target, f.opacity + step);
    } else if (f.opacity > f.target) {
        f.opacity = std::max(f.target, f.opacity - step);
    }
}

}  // namespace

// ----- Y-axis ---------------------------------------------------------------

void update_y_fades(VroomChart& chart,
                    const Layout& lay,
                    const PriceBounds& bounds) {
    const double range = bounds.max - bounds.min;
    if (range <= 0.0) return;

    const float candle_area_h = chart.height_px - lay.x_axis_height_px;
    const double interval = vroom::pick_price_interval(range, candle_area_h);
    if (interval <= 0.0) return;

    for (auto& f : chart.y_fades) f.target = 0.f;

    const double first = std::ceil(bounds.min / interval) * interval;
    constexpr int kMaxLabels = 64;
    int promoted = 0;
    for (double price = first;
         price <= bounds.max && promoted < kMaxLabels;
         price += interval, ++promoted) {
        bool found = false;
        for (auto& f : chart.y_fades) {
            if (price_matches(f.price, price)) {
                f.target = 1.f;
                found = true;
                break;
            }
        }
        if (!found) {
            chart.y_fades.push_back(YLabelFade{price, 0.f, 1.f});
        }
    }

    const float step = kFadeRate * chart.last_dt_seconds;
    for (auto& f : chart.y_fades) advance(f, step, chart.last_dt_seconds);
}

void draw_y_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      const Layout& lay,
                      const PriceBounds& bounds,
                      float candle_right,
                      float candle_area_h) {
    SkPaint grid;
    grid.setColor(chart.theme.colors[VROOM_COLOR_GRID]);
    grid.setStrokeWidth(1.f);
    grid.setAntiAlias(true);
    for (const auto& f : chart.y_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float y = vroom::price_to_y(lay, bounds, f.price);
        if (y < 0.f || y > candle_area_h) continue;
        grid.setAlphaf(f.opacity);
        canvas->drawLine(0.f, y, candle_right, y, grid);
    }
}

void draw_y_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay,
                   const PriceBounds& bounds) {
    auto tf = vroom::axis_typeface();
    if (!tf) return;

    const float candle_area_h = chart.height_px - lay.x_axis_height_px;

    SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    const float cap_h = metrics.fCapHeight > 0
        ? metrics.fCapHeight
        : -metrics.fAscent * 0.7f;

    SkPaint text_paint;
    text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
    text_paint.setAntiAlias(true);

    constexpr float kRightInset = 6.f;

    for (const auto& f : chart.y_fades) {
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

void gc_y_fades(VroomChart& chart) {
    chart.y_fades.erase(
        std::remove_if(chart.y_fades.begin(), chart.y_fades.end(),
                       [](const YLabelFade& f) {
                           return f.opacity <= 0.f && f.target <= 0.f;
                       }),
        chart.y_fades.end());
}

// ----- X-axis ---------------------------------------------------------------

void update_x_fades(VroomChart& chart, const Layout& lay) {
    if (chart.candles.empty()) return;
    if (chart.visible_end_ms <= chart.visible_start_ms) return;

    const float candle_area_w =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    if (candle_area_w <= 0.f) return;

    const int64_t window_ms = chart.visible_end_ms - chart.visible_start_ms;
    const int64_t interval = vroom::pick_time_interval(window_ms, candle_area_w);
    if (interval <= 0) return;

    for (auto& f : chart.x_fades) f.target = 0.f;

    int64_t t = (chart.visible_start_ms / interval) * interval;
    if (t < chart.visible_start_ms) t += interval;
    constexpr int kMaxLabels = 64;
    int promoted = 0;
    for (; t <= chart.visible_end_ms && promoted < kMaxLabels;
         t += interval, ++promoted) {
        bool found = false;
        for (auto& f : chart.x_fades) {
            if (f.time_ms == t) {
                f.target = 1.f;
                found = true;
                break;
            }
        }
        if (!found) {
            chart.x_fades.push_back(XLabelFade{t, 0.f, 1.f});
        }
    }

    const float step = kFadeRate * chart.last_dt_seconds;
    for (auto& f : chart.x_fades) advance(f, step, chart.last_dt_seconds);
}

void draw_x_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      float candle_area_w,
                      float candle_area_h) {
    if (chart.visible_end_ms <= chart.visible_start_ms) return;
    const int64_t window_ms = chart.visible_end_ms - chart.visible_start_ms;
    if (window_ms <= 0 || candle_area_w <= 0.f) return;

    SkPaint grid;
    grid.setColor(chart.theme.colors[VROOM_COLOR_GRID]);
    grid.setStrokeWidth(1.f);
    grid.setAntiAlias(true);
    for (const auto& f : chart.x_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float frac =
            static_cast<float>(f.time_ms - chart.visible_start_ms) /
            static_cast<float>(window_ms);
        const float x = frac * candle_area_w;
        if (x < 0.f || x > candle_area_w) continue;
        grid.setAlphaf(f.opacity);
        canvas->drawLine(x, 0.f, x, candle_area_h, grid);
    }
}

void draw_x_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay) {
    auto tf = vroom::axis_typeface();
    if (!tf || chart.candles.empty()) return;
    if (chart.visible_end_ms <= chart.visible_start_ms) return;

    const float candle_area_h = chart.height_px - lay.x_axis_height_px;
    const float candle_area_w =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    if (candle_area_w <= 0.f) return;

    const int64_t window_ms = chart.visible_end_ms - chart.visible_start_ms;
    // Recompute interval here just to decide HH:MM vs MM/DD formatting.
    const int64_t interval = vroom::pick_time_interval(window_ms, candle_area_w);

    SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    const float cap_h = metrics.fCapHeight > 0
        ? metrics.fCapHeight
        : -metrics.fAscent * 0.7f;

    SkPaint text_paint;
    text_paint.setColor(chart.theme.colors[VROOM_COLOR_AXIS_TEXT]);
    text_paint.setAntiAlias(true);

    const float baseline_y =
        candle_area_h + (lay.x_axis_height_px + cap_h) * 0.5f;
    const bool use_date = interval >= 24LL * 60 * 60 * 1000;
    for (const auto& f : chart.x_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float frac =
            static_cast<float>(f.time_ms - chart.visible_start_ms) /
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

void gc_x_fades(VroomChart& chart) {
    chart.x_fades.erase(
        std::remove_if(chart.x_fades.begin(), chart.x_fades.end(),
                       [](const XLabelFade& f) {
                           return f.opacity <= 0.f && f.target <= 0.f;
                       }),
        chart.x_fades.end());
}

// ----- Axis-width sizing ----------------------------------------------------

void recompute_axis_width(VroomChart& chart) {
    auto tf = vroom::axis_typeface();
    if (!tf) {
        chart.axis_width_px = 0.f;
        return;
    }
    double hi, lo;
    if (chart.price_bounds_initialized) {
        hi = chart.price_bounds.max;
        lo = chart.price_bounds.min;
    } else if (!chart.candles.empty()) {
        hi = chart.candles.front().high;
        lo = chart.candles.front().low;
        for (const auto& c : chart.candles) {
            if (c.high > hi) hi = c.high;
            if (c.low < lo) lo = c.low;
        }
    } else {
        chart.axis_width_px = 0.f;
        return;
    }

    SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
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
    chart.axis_width_px = text_w + kPaddingLeft + kPaddingRight;
}

}  // namespace vroom::labels
