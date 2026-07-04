#include "drawings.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include "chart.h"

namespace vroom::drawings {

namespace {
// Node-dot styling (fixed for v1, not themed): a 4px black dot with a 2px blue
// ring around it. kNodeFillRadius is the black core; the blue stroke is centered
// at kNodeRingRadius so it sits just outside the core.
constexpr SkColor kNodeFill = 0xff000000;       // black core
constexpr SkColor kNodeBorder = 0xff2962ff;     // blue ring
constexpr float   kNodeFillRadius = 2.f;        // 4px diameter
constexpr float   kNodeBorderWidth = 2.f;
constexpr float   kNodeRingRadius = 3.f;        // ring centered outside the core

// Maps a data-space point to pixels in the price pane.
SkPoint to_px(const VroomChart& chart, const Layout& lay, const PriceBounds& bounds,
              int64_t window_ms, const VroomDrawPoint& p) {
    const float x = vroom::x_at_time(lay, chart.visible_start_ms, window_ms, p.time_ms);
    const float y = vroom::price_to_y(lay, bounds, p.price);
    return SkPoint{x, y};
}

void draw_node(SkCanvas* canvas, SkPoint pt) {
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(kNodeFill);
    canvas->drawCircle(pt.fX, pt.fY, kNodeFillRadius, fill);

    SkPaint border;
    border.setAntiAlias(true);
    border.setColor(kNodeBorder);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(kNodeBorderWidth);
    canvas->drawCircle(pt.fX, pt.fY, kNodeRingRadius, border);
}
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    const int64_t window_ms = chart.visible_end_ms - chart.visible_start_ms;
    if (window_ms <= 0) return;

    const SkRect clip = SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h);

    // 1. Committed line segments, clipped to the candle area.
    if (!chart.drawings.empty()) {
        canvas->save();
        canvas->clipRect(clip);
        for (const VroomDrawing& d : chart.drawings) {
            const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
            const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
            SkPaint line;
            line.setAntiAlias(true);
            line.setColor(static_cast<SkColor>(d.color));
            line.setStyle(SkPaint::kStroke_Style);
            line.setStrokeWidth(d.width > 0.f ? d.width : 2.f);
            canvas->drawLine(a, b, line);
        }
        canvas->restore();
    }

    if (!chart.draft_active) return;

    const SkPoint a = to_px(chart, lay, bounds, window_ms, chart.draft_a);
    const bool has_b = chart.draft_has_b;
    const SkPoint b =
        has_b ? to_px(chart, lay, bounds, window_ms, chart.draft_b) : a;

    // 2. Guideline preview (A->B), clipped to the candle area. Only while the
    //    second point is still being placed (draft_guide); once committed, the
    //    solid segment comes from chart.drawings instead.
    if (chart.draft_guide && has_b) {
        canvas->save();
        canvas->clipRect(clip);
        SkPaint guide;
        guide.setAntiAlias(true);
        guide.setColor(static_cast<SkColor>(chart.draft_color));
        guide.setStyle(SkPaint::kStroke_Style);
        guide.setStrokeWidth(chart.draft_width > 0.f ? chart.draft_width : 2.f);
        canvas->drawLine(a, b, guide);
        canvas->restore();
    }

    // 3. Node dots on top (not clipped, so an edge dot still renders fully).
    //    While guiding (placing the second point) only the anchor dot shows; the
    //    moving end is conveyed by the guideline. Once selected (committed) both
    //    endpoints show dots.
    draw_node(canvas, a);
    if (has_b && !chart.draft_guide) draw_node(canvas, b);
}

}  // namespace vroom::drawings
