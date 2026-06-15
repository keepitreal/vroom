#include "crosshair.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include <algorithm>

#include "chart.h"
#include "theme.h"

namespace vroom::crosshair {

namespace {
constexpr float kRingRadius = 3.5f;  // hollow dot at the intersection
constexpr SkScalar kDash[2] = {2.f, 2.f};
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          float candle_right,
          float candle_area_h,
          float vline_bottom,
          float snap_x) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;

    // Vertical line + ring snap to the nearest candle's center x; the horizontal
    // line and the ring's y follow the (lifted) touch y. Clamp into the candle
    // area so nothing bleeds into the axis strips.
    const float cx = std::clamp(snap_x, 0.f, candle_right);
    const float cy = std::clamp(chart.crosshair_y_px, 0.f, candle_area_h);

    const SkColor color = chart.theme.colors[VROOM_COLOR_CROSSHAIR];

    // Dashed perpendicular lines. The vertical line runs the full height of the
    // candle + indicator region (down to vline_bottom) so it stays visible over
    // any below-chart panes; the horizontal line spans from the left edge to the
    // y-axis strip at candle_right.
    SkPaint dash;
    dash.setAntiAlias(true);
    dash.setColor(color);
    dash.setStrokeWidth(1.f);
    dash.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(cx, 0.f, cx, vline_bottom, dash);
    canvas->drawLine(0.f, cy, candle_right, cy, dash);

    // Punch the dashes out from under the ring so its center reads as hollow.
    SkPaint hole;
    hole.setAntiAlias(true);
    hole.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawCircle(cx, cy, kRingRadius, hole);

    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setColor(chart.theme.colors[VROOM_COLOR_CROSSHAIR_TARGET]);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(2.f);  // thicker border so the dot reads clearly
    canvas->drawCircle(cx, cy, kRingRadius, ring);
}

}  // namespace vroom::crosshair
