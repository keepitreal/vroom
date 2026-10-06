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
#include <vector>

#include "chart.h"
#include "fonts.h"
#include "price_format.h"
#include "price_line_layout.h"
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

// What the asset is worth, which is what its price precision keys off. The
// newest close rather than the visible range, so panning and zooming don't
// shift the decimals around under the user.
//
// An asset that has crossed orders of magnitude within one series (a token at
// 0.00001 that later trades at 100) reads its recent scale here, so deep
// history would round flat — the tick guard at each label site is what rescues
// it, since the bounds down there are tiny enough to demand more decimals.
double reference_price(const VroomChart& chart) {
    if (!chart.candles.empty()) return chart.candles.back().close;
    return (chart.price_bounds.max + chart.price_bounds.min) * 0.5;
}

// Drives a whole axis off the morph envelope instead of the per-label fades:
// everything in the phase's tick set shares one opacity, and anything outside it
// is dropped outright. At the midpoint the axis is transparent, so swapping the
// set there costs nothing visually.
template <typename Fade>
void apply_envelope(std::vector<Fade>& fades, float opacity) {
    for (auto& f : fades) f.opacity = f.target > 0.f ? opacity : 0.f;
}

}  // namespace

IntervalPhase interval_phase(const VroomChart& chart) {
    if (chart.morph_from.empty() || chart.interval_morph_t >= 1.f) return {};
    // A live update keeps the same interval, so its ticks are still the right
    // ones — they stay put and keep their own per-label fades.
    if (chart.morph_is_stream) return {};
    const float t = chart.interval_morph_t;
    constexpr float kMid = 0.5f;
    if (t < kMid) return {true, true, 1.f - t / kMid};
    return {true, false, (t - kMid) / (1.f - kMid)};
}

// ----- Y-axis ---------------------------------------------------------------

void update_y_fades(VroomChart& chart,
                    const Layout& lay,
                    const PriceBounds& bounds) {
    if (!(bounds.max > bounds.min)) return;

    const float candle_area_h = vroom::price_pane_bottom(lay);
    constexpr int kMaxLabels = 64;
    std::vector<double> ticks;
    vroom::price_ticks(bounds, candle_area_h, kMaxLabels, ticks);
    if (ticks.empty()) return;

    for (auto& f : chart.y_fades) f.target = 0.f;

    for (const double price : ticks) {
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

    const auto phase = interval_phase(chart);
    if (phase.active) {
        apply_envelope(chart.y_fades, phase.opacity);
        return;
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
    // The fade state still advances in update_y_fades, so a hidden axis picks up
    // wherever the ticks moved to once it is revealed again.
    if (lay.y_axis_opacity <= 0.f) return;

    const float candle_area_h = vroom::price_pane_bottom(lay);

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

    // Horizontal center of the y-axis container ([width - y_axis_width, width]).
    // Labels (and the price box) center on this so their text shares a column.
    const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
    const vroom::PriceFormat fmt = vroom::with_tick_guard(
        chart.price_fmt, vroom::price_label_interval(bounds, candle_area_h));

    for (const auto& f : chart.y_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float y = vroom::price_to_y(lay, bounds, f.price);

        char buf[48];
        vroom::format_price(buf, sizeof(buf), f.price, fmt);
        const size_t len = std::strlen(buf);
        const float text_w = font.measureText(
            buf, len, SkTextEncoding::kUTF8);

        const float text_x = axis_center_x - text_w * 0.5f;
        const float baseline_y = y + cap_h * 0.5f;
        if (baseline_y - cap_h < 0.f) continue;
        if (baseline_y > candle_area_h) continue;

        text_paint.setAlphaf(f.opacity * lay.y_axis_opacity);
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

void update_x_fades(VroomChart& chart,
                    const Layout& lay,
                    int64_t start_ms,
                    int64_t end_ms) {
    if (chart.candles.empty()) return;
    if (end_ms <= start_ms) return;

    const float candle_area_w =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    if (candle_area_w <= 0.f) return;

    const int64_t window_ms = end_ms - start_ms;
    const vroom::TimeTick tick =
        vroom::pick_time_tick(window_ms, candle_area_w);

    for (auto& f : chart.x_fades) f.target = 0.f;

    int64_t t = vroom::first_tick_at_or_after(start_ms, tick);
    constexpr int kMaxLabels = 64;
    int promoted = 0;
    for (; t <= end_ms && promoted < kMaxLabels;
         t = vroom::next_tick(t, tick), ++promoted) {
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

    const auto phase = interval_phase(chart);
    if (phase.active) {
        apply_envelope(chart.x_fades, phase.opacity);
        return;
    }
    const float step = kFadeRate * chart.last_dt_seconds;
    for (auto& f : chart.x_fades) advance(f, step, chart.last_dt_seconds);
}

void draw_x_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      int64_t start_ms,
                      int64_t end_ms,
                      float candle_area_w,
                      float candle_area_h) {
    if (end_ms <= start_ms) return;
    const int64_t window_ms = end_ms - start_ms;
    if (window_ms <= 0 || candle_area_w <= 0.f) return;

    SkPaint grid;
    grid.setColor(chart.theme.colors[VROOM_COLOR_GRID]);
    grid.setStrokeWidth(1.f);
    grid.setAntiAlias(true);
    for (const auto& f : chart.x_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float frac = static_cast<float>(f.time_ms - start_ms) /
                           static_cast<float>(window_ms);
        const float x = frac * candle_area_w;
        if (x < 0.f || x > candle_area_w) continue;
        grid.setAlphaf(f.opacity);
        canvas->drawLine(x, 0.f, x, candle_area_h, grid);
    }
}

void draw_x_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay,
                   int64_t start_ms,
                   int64_t end_ms) {
    auto tf = vroom::axis_typeface();
    if (!tf || chart.candles.empty()) return;
    if (end_ms <= start_ms) return;
    // See draw_y_labels: update_x_fades keeps running while the strip is hidden.
    if (lay.x_axis_opacity <= 0.f) return;

    // X-axis labels live in the bottom strip, which stays anchored regardless
    // of any indicator pane above it.
    const float candle_area_h = vroom::x_axis_top(lay);
    const float candle_area_w =
        lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    if (candle_area_w <= 0.f) return;

    const int64_t window_ms = end_ms - start_ms;
    // Recompute the cadence here just to decide label formatting.
    const vroom::TimeTick tick =
        vroom::pick_time_tick(window_ms, candle_area_w);

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
    const bool use_date =
        tick.unit == vroom::TimeUnit::Fixed &&
        tick.step_ms >= 24LL * 60 * 60 * 1000;
    for (const auto& f : chart.x_fades) {
        if (f.opacity <= 1e-3f) continue;
        const float frac = static_cast<float>(f.time_ms - start_ms) /
                           static_cast<float>(window_ms);
        const float x_center = frac * candle_area_w;

        char buf[16];
        const time_t time_s = static_cast<time_t>(f.time_ms / 1000);
        struct tm tm_buf;
        localtime_r(&time_s, &tm_buf);
        if (tick.unit == vroom::TimeUnit::Year) {
            std::snprintf(buf, sizeof(buf), "%d", tm_buf.tm_year + 1900);
        } else if (tick.unit == vroom::TimeUnit::Month) {
            // Show the year at January so month ticks read across years.
            if (tm_buf.tm_mon == 0) {
                std::snprintf(buf, sizeof(buf), "%d", tm_buf.tm_year + 1900);
            } else {
                std::strftime(buf, sizeof(buf), "%b", &tm_buf);
            }
        } else if (use_date) {
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

        text_paint.setAlphaf(f.opacity * lay.x_axis_opacity);
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

namespace {

// The scale the y labels will actually draw against: the pre-switch scale while
// an interval morph is still showing the old ticks, otherwise the manual scale
// or the visible candles' auto-fit. False when there is nothing to lay out.
bool label_scale(const VroomChart& chart, vroom::PriceBounds* out) {
    if (interval_phase(chart).outgoing) {
        *out = chart.morph_from_bounds;
        return out->max > out->min;
    }
    if (chart.price_bounds_manual) {
        *out = chart.price_bounds;
        return out->max > out->min;
    }
    if (chart.candles.empty()) return false;
    const auto idx = vroom::visible_indices(
        chart.candles.data(), chart.candles.size(),
        chart.visible_start_ms, chart.visible_end_ms);
    if (idx.end <= idx.start) return false;
    *out = vroom::auto_price_bounds(
        chart.candles.data() + idx.start, idx.end - idx.start,
        chart.price_bounds.log);
    return out->max > out->min;
}

float measure_price(const SkFont& font, double price, const vroom::PriceFormat& fmt) {
    char buf[48];
    vroom::format_price(buf, sizeof(buf), price, fmt);
    return font.measureText(buf, std::strlen(buf), SkTextEncoding::kUTF8);
}

}  // namespace

static_assert(kAxisBadgePadH == vroom::price_lines::kPadH,
              "the y-axis strip budgets the badge pad the pills actually draw");

void recompute_axis_width(VroomChart& chart) {
    // Precision first, and unconditionally: the label sites read it whether or
    // not a typeface has loaded, and it's what the measurement below sizes for.
    chart.price_fmt = vroom::price_format_for(reference_price(chart));

    auto tf = vroom::axis_typeface();
    if (!tf) {
        chart.axis_width_px = 0.f;
        return;
    }

    vroom::PriceBounds bounds{};
    const bool have_scale = label_scale(chart, &bounds);
    if (!have_scale) {
        // Still size for a badge that draws without a scale (the latest close,
        // or a price line parked on an empty chart).
        if (!chart.candles.empty()) {
            const double p = chart.candles.back().close;
            bounds = {p, p};
        } else if (!chart.price_lines.empty()) {
            const double p = chart.price_lines.front().price;
            bounds = {p, p};
        } else {
            chart.axis_width_px = 0.f;
            return;
        }
    }

    const auto lay = chart.layout();
    // Same guard the label sites apply, against the bounds this is sizing for,
    // so a zoom deep enough to add decimals widens the strip to hold them.
    const vroom::PriceFormat fmt = vroom::with_tick_guard(
        chart.price_fmt,
        vroom::price_label_interval(bounds, vroom::price_pane_bottom(lay)));

    const float axis_px = chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX];
    const float badge_px = vroom::badge_font_px(chart.theme);
    const float requested = chart.price_line_style.font_size_px > 0.f
                                ? chart.price_line_style.font_size_px
                                : badge_px;
    SkFont axis_font(tf, axis_px);
    SkFont badge_font(tf, badge_px);
    SkFont line_font(tf, vroom::price_lines::clamp_label_font_px(requested));

    // Tick text and the crosshair / current-price badges share the column, so
    // budget the wider of the two at the badge pad.
    float axis_w = 0.f;
    float line_w = 0.f;
    const auto consider = [&](double price) {
        if (!std::isfinite(price)) return;
        axis_w = std::max({axis_w, measure_price(axis_font, price, fmt),
                           measure_price(badge_font, price, fmt)});
        line_w = std::max(line_w, measure_price(line_font, price, fmt));
    };

    consider(bounds.min);
    consider(bounds.max);
    if (!chart.candles.empty()) consider(chart.candles.back().close);
    for (const auto& pl : chart.price_lines) consider(pl.price);
    if (chart.dragged_price_line >= 0) consider(chart.dragged_price_line_price);

    // A nice tick can be wider than either endpoint ("100,000.00" above a
    // 99,950 high). Same walk update_y_fades uses, capped the same way.
    constexpr int kMaxLabels = 64;
    std::vector<double> ticks;
    vroom::price_ticks(bounds, vroom::price_pane_bottom(lay), kMaxLabels, ticks);
    for (const double price : ticks) consider(price);

    chart.axis_width_px = axis_strip_width(axis_content_width(axis_w, line_w));
}

}  // namespace vroom::labels
