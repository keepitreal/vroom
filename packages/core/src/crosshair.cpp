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

// Scales each RGB channel by `factor` (clamped), preserving alpha. Used to
// lighten the ring relative to the crosshair lines without a separate theme key.
SkColor lighten(SkColor c, float factor) {
    auto scale = [factor](U8CPU v) -> U8CPU {
        const float s = v * factor + 0.5f;
        return static_cast<U8CPU>(s > 255.f ? 255.f : s);
    };
    return SkColorSetARGB(SkColorGetA(c), scale(SkColorGetR(c)),
                          scale(SkColorGetG(c)), scale(SkColorGetB(c)));
}
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          float candle_right,
          float candle_area_h) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;

    // The intersection follows the touch point (already lifted above the thumb
    // on the JS side). Clamp into the candle area so the dot never lands in the
    // axis strips.
    const float cx = std::clamp(chart.crosshair_x_px, 0.f, candle_right);
    const float cy = std::clamp(chart.crosshair_y_px, 0.f, candle_area_h);

    const SkColor color = chart.theme.colors[VROOM_COLOR_CROSSHAIR];

    // Dashed perpendicular lines. Vertical spans the full chart height down to
    // the x-axis separator; horizontal spans from the left edge to the y-axis
    // separator at candle_right.
    SkPaint dash;
    dash.setAntiAlias(true);
    dash.setColor(color);
    dash.setStrokeWidth(1.f);
    dash.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(cx, 0.f, cx, candle_area_h, dash);
    canvas->drawLine(0.f, cy, candle_right, cy, dash);

    // Punch the dashes out from under the ring so its center reads as hollow.
    SkPaint hole;
    hole.setAntiAlias(true);
    hole.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawCircle(cx, cy, kRingRadius, hole);

    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setColor(lighten(color, 1.3f));  // 30% lighter than the lines
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(2.f);  // thicker border so the dot reads clearly
    canvas->drawCircle(cx, cy, kRingRadius, ring);
}

}  // namespace vroom::crosshair
