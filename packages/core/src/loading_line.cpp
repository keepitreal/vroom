#include "loading_line.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cstddef>

#include "chart.h"
#include "loading_wave.h"
#include "viewport.h"

namespace vroom::loading_line {

namespace {

constexpr float kStrokeWidthPx = 2.f;

// Spacing of the idle curve's vertices. Fine enough that a polyline reads as a
// smooth sine, coarse enough not to rebuild a thousand-point path every frame.
constexpr float kIdleSampleStepPx = 3.f;

// Multiplies into whatever alpha the theme colour already carries, so a
// deliberately translucent skeleton colour stays translucent.
SkColor faded(SkColor c, float a) {
    const float scaled = static_cast<float>(SkColorGetA(c)) *
                         std::clamp(a, 0.f, 1.f);
    return SkColorSetA(c, static_cast<U8CPU>(scaled + 0.5f));
}

void stroke_path(SkCanvas* canvas,
                 const VroomChart& chart,
                 const SkPath& path,
                 float alpha) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(kStrokeWidthPx);
    // Round join over miter: the morph target is a polyline through real price
    // data, and a spiky series would throw miter spikes off the crests.
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setAntiAlias(true);
    p.setColor(faded(chart.theme.colors[VROOM_COLOR_SKELETON], alpha));
    canvas->drawPath(path, p);
}

// Stage 1: the sine alone, sampled across the whole plot.
SkPath idle_path(const VroomChart& chart, float area_w, float pane_h) {
    const int samples = std::clamp(
        static_cast<int>(area_w / kIdleSampleStepPx), 16, 512);
    // Pinned to phase 0 under reduced motion, which leaves a still curve rather
    // than nothing — the placeholder is still worth showing.
    const float elapsed = chart.loading_animate ? chart.loading_elapsed_s : 0.f;

    // SkPathBuilder, not SkPath's edit methods — removed in newer Skia tips.
    SkPathBuilder path;
    for (int i = 0; i <= samples; ++i) {
        const float xf = static_cast<float>(i) / static_cast<float>(samples);
        const float x = xf * area_w;
        const float y = loading_wave::y_frac(elapsed, xf) * pane_h;
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    return path.detach();
}

// Stages 2 and 3: the captured vertices, eased from the frozen sine toward the
// candle centres. `t` of 1 is the shape the candles grow out of.
SkPath morph_path(const VroomChart& chart,
                  const Layout& lay,
                  float area_w,
                  float pane_h,
                  float t) {
    SkPathBuilder path;
    for (std::size_t i = 0; i < chart.loading_line.size(); ++i) {
        const LinePoint& pt = chart.loading_line[i];
        const float from_y = pt.from_y * pane_h;
        const float to_y = y_at_fraction(lay, pt.to_y);
        const float x = pt.x * area_w;
        const float y = from_y + (to_y - from_y) * t;
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    return path.detach();
}

}  // namespace

void draw(SkCanvas* canvas, const VroomChart& chart, const Layout& lay) {
    if (!canvas) return;
    const float area_w = candle_area_width(lay);
    const float pane_h = price_pane_bottom(lay);
    if (area_w <= 0.f || pane_h <= 0.f) return;

    // A capture present means the data has landed and the line is on its way to
    // the candle centres; absent means nothing has arrived yet.
    const SkPath path = chart.loading_line.empty()
        ? idle_path(chart, area_w, pane_h)
        : morph_path(chart, lay, area_w, pane_h, chart.loading_line_t);
    stroke_path(canvas, chart, path, 1.f);
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
