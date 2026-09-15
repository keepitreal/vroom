#include "loading_skeleton.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>

#include "chart.h"
#include "loading_series.h"
#include "loading_wave.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::loading_skeleton {

namespace {

// Body corner radius. Deliberately not VROOM_FLOAT_CANDLE_RADIUS_PX: that
// defaults to 0, and square bars read as data. Rounded ones read as chrome,
// which is the whole point of a skeleton.
constexpr float kBodyRadiusPx = 4.f;
constexpr float kWickWidthPx = 2.f;

// Axis placeholder pills — the stand-ins for the suppressed tick labels.
constexpr float kPillW = 52.f;
constexpr float kPillH = 12.f;
constexpr float kPillR = 6.f;
constexpr float kPillInsetPx = 10.f;
// Fractions of the price pane the y-axis pills sit at.
constexpr float kYPillAt[] = {0.2f, 0.5f, 0.8f};
constexpr int kXPillCount = 5;

// Bar-count guard rails. The low end keeps the wave from degenerating into a
// couple of bars flapping; the high end caps the per-frame cost on a wide
// canvas and stops the bodies collapsing to sub-pixel slivers.
constexpr std::size_t kMinBars = 8;
constexpr std::size_t kMaxBars = loading_series::kLength;

// Replaces a color's alpha outright rather than scaling it — the wave owns the
// alpha channel here, and the theme color arrives opaque.
inline SkColor with_alpha(uint32_t argb, float a) {
    const auto alpha = static_cast<uint32_t>(
        std::clamp(a, 0.f, 1.f) * 255.f + 0.5f);
    return static_cast<SkColor>((alpha << 24) | (argb & 0x00FFFFFFu));
}

}  // namespace

std::size_t bar_count(const VroomChart& chart, const Layout& lay) {
    const float area_w = vroom::candle_area_width(lay);
    if (area_w <= 0.f) return 0;
    // Match the width the real series is about to land at, so the bars don't
    // change size across the hand-off.
    const float step = chart.default_candle_px > 0.f ? chart.default_candle_px : 8.f;
    const auto n = static_cast<std::size_t>(area_w / step);
    return std::clamp(n, kMinBars, kMaxBars);
}

void draw(SkCanvas* canvas, const VroomChart& chart, const Layout& lay) {
    if (!canvas) return;

    const float area_w = vroom::candle_area_width(lay);
    const float candle_area_h = vroom::price_pane_bottom(lay);
    const float candle_right = lay.width_px - lay.y_axis_width_px - lay.right_padding_px;
    if (area_w <= 0.f || candle_area_h <= 0.f) return;

    const std::size_t n = bar_count(chart, lay);
    if (n == 0) return;
    const ::VroomCandle* bars = loading_series::tail(chart.loading_candles, n);
    if (!bars) return;

    const auto bounds = vroom::auto_price_bounds(bars, n);
    const uint32_t grey = chart.theme.colors[VROOM_COLOR_SKELETON];
    // Frozen at phase 0 under reduced motion: a still skeleton, and nothing
    // pinning a render loop for someone who asked for less movement.
    const float elapsed = chart.loading_animate ? chart.loading_elapsed_s : 0.f;

    // Gridlines, so the pane keeps its structure while the data is missing.
    // Placed at the same fractions as the y-axis pills below rather than routed
    // through labels::draw_y_gridlines: that one renders the tick set built by
    // update_y_fades, which would peg the lines to the placeholder walk's
    // arbitrary prices *and* leave tick state behind for the real series to
    // reconcile. Fixed fractions keep each line paired with its pill, which is
    // what makes the two read as one axis.
    SkPaint grid;
    grid.setColor(chart.theme.colors[VROOM_COLOR_GRID]);
    grid.setStrokeWidth(1.f);
    grid.setAntiAlias(true);
    for (float at : kYPillAt) {
        const float y = candle_area_h * at;
        canvas->drawLine(0.f, y, candle_right, y, grid);
    }

    // Volume occupies the same band the real bars will, so the hand-off doesn't
    // reflow the pane — and it's skipped entirely when the chart it's standing
    // in for won't have any, so the skeleton doesn't promise a layer the real
    // series then lacks.
    const float vol_frac = chart.volume.height_frac >= 0.f
                               ? chart.volume.height_frac
                               : 0.20f;
    const float vol_region_h =
        chart.volume.enabled != 0 ? candle_area_h * vol_frac : 0.f;
    double max_vol = 0.0;
    for (std::size_t i = 0; i < n; ++i) max_vol = std::max(max_vol, bars[i].volume);

    // One stride for everything, so bodies and volume bars share a grid.
    const float step = area_w / static_cast<float>(n);
    const float body_w = std::max(1.f, step * lay.candle_width_ratio);
    const float half_body = body_w * 0.5f;

    SkPaint body;
    body.setAntiAlias(true);
    SkPaint wick;
    wick.setAntiAlias(true);
    wick.setStyle(SkPaint::kStroke_Style);
    wick.setStrokeWidth(kWickWidthPx);

    // The wave scales bars up to 1.5x, which the price bounds know nothing
    // about — a tall bar near the top of its range can crest past the pane. The
    // settled chart needs no clip because its bars are inside the bounds by
    // construction; this one does.
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(candle_right, candle_area_h));

    for (std::size_t i = 0; i < n; ++i) {
        const ::VroomCandle& c = bars[i];
        const float cx = (static_cast<float>(i) + 0.5f) * step;
        // Bars fully off the plot are skipped rather than clamped, so a partial
        // column never leaves a stub pinned to the edge.
        if (cx + half_body < 0.f || cx - half_body > candle_right) continue;

        const auto w = loading_wave::at(elapsed, static_cast<int>(i));
        const SkColor tint = with_alpha(grey, w.alpha);
        body.setColor(tint);
        wick.setColor(tint);

        const float y_high = vroom::price_to_y(lay, bounds, c.high);
        const float y_low = vroom::price_to_y(lay, bounds, c.low);
        const float y_open = vroom::price_to_y(lay, bounds, c.open);
        const float y_close = vroom::price_to_y(lay, bounds, c.close);

        // Both the wick and the body scale about their own midpoint, so each
        // breathes symmetrically instead of growing off one end — which would
        // read as the series jittering rather than pulsing.
        const float wick_mid = (y_high + y_low) * 0.5f;
        const float wick_half = (y_low - y_high) * 0.5f * w.scale;
        canvas->drawLine(cx, wick_mid - wick_half, cx, wick_mid + wick_half, wick);

        const float body_top = std::min(y_open, y_close);
        const float body_bot = std::max(y_open, y_close);
        const float body_mid = (body_top + body_bot) * 0.5f;
        const float body_h = std::max(1.f, (body_bot - body_top) * w.scale);
        const SkRect rect = SkRect::MakeXYWH(cx - half_body,
                                            body_mid - body_h * 0.5f,
                                            body_w, body_h);
        const float r = std::min({kBodyRadiusPx, body_w * 0.5f, body_h * 0.5f});
        if (r > 0.f) {
            canvas->drawRoundRect(rect, r, r, body);
        } else {
            canvas->drawRect(rect, body);
        }

        // Volume rides the same wave as the candle above it. The reference
        // implementation left it at full strength, which reads as one real
        // layer under a placeholder one.
        if (vol_region_h > 0.f && max_vol > 0.0) {
            const auto frac = static_cast<float>(c.volume / max_vol);
            const float vh = std::max(1.f, frac * vol_region_h * w.scale);
            canvas->drawRect(
                SkRect::MakeXYWH(cx - half_body, candle_area_h - vh, body_w, vh),
                body);
        }
    }

    canvas->restore();

    // Axis strip backgrounds, as in the settled frame: they mask any bar that
    // overshot the plot into the label gutters.
    SkPaint axis_bg;
    axis_bg.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawRect(SkRect::MakeXYWH(0, vroom::x_axis_top(lay), candle_right,
                                      lay.x_axis_height_px),
                     axis_bg);
    canvas->drawRect(SkRect::MakeXYWH(candle_right, 0,
                                      lay.width_px - candle_right, lay.height_px),
                     axis_bg);

    // Placeholder pills where the tick labels would be. An axis stripped of its
    // text but left empty reads as broken rather than as pending.
    SkPaint pill;
    pill.setAntiAlias(true);
    pill.setColor(with_alpha(grey, loading_wave::kMinAlpha));

    if (lay.y_axis_opacity > 0.f && lay.y_axis_width_px > 0.f) {
        const float x = lay.width_px - kPillInsetPx - kPillW;
        for (float at : kYPillAt) {
            const float cy = candle_area_h * at;
            canvas->drawRoundRect(
                SkRect::MakeXYWH(x, cy - kPillH * 0.5f, kPillW, kPillH),
                kPillR, kPillR, pill);
        }
    }

    if (lay.x_axis_opacity > 0.f && lay.x_axis_height_px > 0.f) {
        const float cy = vroom::x_axis_top(lay) + lay.x_axis_height_px * 0.5f;
        // Evenly spaced across the plot, centered in their slots, mirroring how
        // the real time labels distribute.
        const float slot = candle_right / static_cast<float>(kXPillCount);
        for (int i = 0; i < kXPillCount; ++i) {
            const float cx = slot * (static_cast<float>(i) + 0.5f);
            canvas->drawRoundRect(
                SkRect::MakeXYWH(cx - kPillW * 0.5f, cy - kPillH * 0.5f,
                                 kPillW, kPillH),
                kPillR, kPillR, pill);
        }
    }
}

}  // namespace vroom::loading_skeleton
