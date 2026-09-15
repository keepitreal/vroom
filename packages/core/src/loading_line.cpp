#include "loading_line.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cstddef>
#include <vector>

#include "chart.h"
#include "curve.h"
#include "loading_wave.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::loading_line {

namespace {

constexpr float kStrokeWidthPx = 2.f;

// Multiplies into whatever alpha the resolved color already carries.
SkColor faded(SkColor c, float a) {
    const float scaled = static_cast<float>(SkColorGetA(c)) *
                         std::clamp(a, 0.f, 1.f);
    return SkColorSetA(c, static_cast<U8CPU>(scaled + 0.5f));
}

// The loading line's color. A transparent `skeleton` inherits the gridline
// color, the same sentinel convention the candle border and wick colors use.
// The gridlines are the chart's existing vocabulary for "structure, not data",
// which is exactly what the line is: drawing it in that tone keeps it from
// reading as a series, without needing to be scaled down to stay quiet.
SkColor stroke_color(const VroomChart& chart) {
    const SkColor c = chart.theme.colors[VROOM_COLOR_SKELETON];
    return SkColorGetA(c) == 0 ? chart.theme.colors[VROOM_COLOR_GRID] : c;
}

// Emits `pts` as a monotone cubic — the same curve the line chart draws (see
// curve.h), so the loading line and the series it hands off to are shaped by
// the same math and the transition has no renderer seam to hide.
//
// Monotone rather than a plain spline matters even here: by the end of the
// morph these vertices *are* real price data, and an unconstrained curve would
// overshoot a sharp reversal and bow outside the candle it is supposed to pass
// through.
SkPath spline(const std::vector<SkPoint>& pts) {
    SkPathBuilder path;
    if (pts.empty()) return path.detach();
    path.moveTo(pts[0]);
    // Below three points there is no interior vertex to fit a tangent to.
    if (pts.size() < 3) {
        for (std::size_t i = 1; i < pts.size(); ++i) path.lineTo(pts[i]);
        return path.detach();
    }

    const auto secant_at = [&](std::size_t i) {
        return curve::secant(pts[i].fX, pts[i].fY,
                             pts[i + 1].fX, pts[i + 1].fY);
    };
    const auto tangent_at = [&](std::size_t i) {
        if (i == 0) return secant_at(0);
        if (i + 1 == pts.size()) return secant_at(pts.size() - 2);
        return curve::monotone_tangent(secant_at(i - 1), secant_at(i));
    };

    for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
        // Flat or backwards spacing has no meaningful tangent, and curving
        // through it would fold the line back on itself.
        if (!(pts[i + 1].fX > pts[i].fX)) {
            path.lineTo(pts[i + 1]);
            continue;
        }
        const curve::Controls c = curve::segment_controls(
            pts[i].fX, pts[i].fY, pts[i + 1].fX, pts[i + 1].fY,
            tangent_at(i), tangent_at(i + 1), 1.f);
        path.cubicTo(c.c1x, c.c1y, c.c2x, c.c2y, pts[i + 1].fX, pts[i + 1].fY);
    }
    return path.detach();
}

void stroke_path(SkCanvas* canvas,
                 const VroomChart& chart,
                 const SkPath& path,
                 float alpha) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(kStrokeWidthPx);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setAntiAlias(true);
    p.setColor(faded(stroke_color(chart), alpha));
    canvas->drawPath(path, p);
}

// Pinned to phase 0 under reduced motion, which leaves a still curve at
// mid-breath rather than nothing — the placeholder is still worth showing.
float phase(const VroomChart& chart) {
    return chart.loading_animate ? chart.loading_elapsed_s : 0.f;
}

// Stage 1: the curve alone, sampled across the whole plot.
SkPath idle_path(const VroomChart& chart, float area_w, float pane_h) {
    const int samples = loading_wave::sample_count(area_w);
    const float elapsed = phase(chart);
    std::vector<SkPoint> pts;
    pts.reserve(static_cast<std::size_t>(samples) + 1);
    for (int i = 0; i <= samples; ++i) {
        const float xf = static_cast<float>(i) / static_cast<float>(samples);
        pts.push_back(SkPoint{xf * area_w,
                              loading_wave::y_frac(elapsed, xf) * pane_h});
    }
    return spline(pts);
}

// Stages 2 and 3: the captured vertices, eased from the frozen curve toward the
// candle centres. `t` of 1 is the shape the candles grow out of.
SkPath morph_path(const VroomChart& chart,
                  const Layout& lay,
                  float area_w,
                  float pane_h,
                  float t) {
    std::vector<SkPoint> pts;
    pts.reserve(chart.loading_line.size());
    for (const LinePoint& pt : chart.loading_line) {
        const float from_y = pt.from_y * pane_h;
        const float to_y = y_at_fraction(lay, pt.to_y);
        pts.push_back(SkPoint{pt.x * area_w, from_y + (to_y - from_y) * t});
    }
    return spline(pts);
}

}  // namespace

void draw(SkCanvas* canvas, const VroomChart& chart, const Layout& lay) {
    if (!canvas) return;
    const float area_w = candle_area_width(lay);
    const float pane_h = price_pane_bottom(lay);
    if (area_w <= 0.f || pane_h <= 0.f) return;

    const float b = loading_wave::breath(phase(chart));

    // A capture present means the data has landed and the line is on its way to
    // the candle centres; absent means nothing has arrived yet.
    if (chart.loading_line.empty()) {
        stroke_path(canvas, chart, idle_path(chart, area_w, pane_h), b);
        return;
    }
    // Settle out of the breath as the line resolves, so it holds still at full
    // strength by the time it reaches the candle centres — subtle at the
    // default gridline tone, but it stops a bright custom color from pulsing
    // while the data lands. Stage 3 then fades from full, where draw_fading
    // picks up.
    const float alpha = b + (1.f - b) * chart.loading_line_t;
    stroke_path(canvas, chart,
                morph_path(chart, lay, area_w, pane_h, chart.loading_line_t),
                alpha);
}

void draw_fading(SkCanvas* canvas,
                 const VroomChart& chart,
                 const Layout& lay,
                 float alpha) {
    if (!canvas || alpha <= 0.f) return;
    if (chart.loading_line.empty()) return;
    const float area_w = candle_area_width(lay);
    const float pane_h = price_pane_bottom(lay);
    if (area_w <= 0.f || pane_h <= 0.f) return;

    // Held at the morph's end state: the line has already reached the centres,
    // and the candles are growing out of exactly these pixels.
    stroke_path(canvas, chart, morph_path(chart, lay, area_w, pane_h, 1.f),
                alpha);
}

}  // namespace vroom::loading_line
