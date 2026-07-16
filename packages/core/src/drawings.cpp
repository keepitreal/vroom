#include "drawings.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>

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

void draw_node(SkCanvas* canvas, SkPoint pt, float scale = 1.f) {
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(kNodeFill);
    canvas->drawCircle(pt.fX, pt.fY, kNodeFillRadius * scale, fill);

    SkPaint border;
    border.setAntiAlias(true);
    border.setColor(kNodeBorder);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(kNodeBorderWidth);
    canvas->drawCircle(pt.fX, pt.fY, kNodeRingRadius * scale, border);
}

// Clamped projection parameter (0..1) of point (px,py) onto segment
// (ax,ay)-(bx,by): 0 at A, 1 at B.
float segment_t(float px, float py, float ax, float ay, float bx, float by) {
    const float dx = bx - ax;
    const float dy = by - ay;
    const float len2 = dx * dx + dy * dy;
    const float t = len2 > 0.f ? ((px - ax) * dx + (py - ay) * dy) / len2 : 0.f;
    return std::clamp(t, 0.f, 1.f);
}

// Shortest distance from point (px,py) to segment (ax,ay)-(bx,by), in px.
float dist_to_segment(float px, float py, float ax, float ay, float bx, float by) {
    const float t = segment_t(px, py, ax, ay, bx, by);
    const float cx = ax + t * (bx - ax);
    const float cy = ay + t * (by - ay);
    const float ex = px - cx;
    const float ey = py - cy;
    return std::sqrt(ex * ex + ey * ey);
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

    // 1b. Handles on the selected committed drawing (unclipped, like the draft's
    //     dots). The grabbed endpoint renders 50% larger.
    if (chart.selected_drawing >= 0 &&
        static_cast<size_t>(chart.selected_drawing) < chart.drawings.size()) {
        const VroomDrawing& d = chart.drawings[chart.selected_drawing];
        const SkPoint sa = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint sb = to_px(chart, lay, bounds, window_ms, d.b);
        draw_node(canvas, sa, chart.grabbed_endpoint == 0 ? 1.5f : 1.f);
        draw_node(canvas, sb, chart.grabbed_endpoint == 1 ? 1.5f : 1.f);
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

HitResult hit_test(const VroomChart& chart,
                   const Layout& lay,
                   const PriceBounds& bounds,
                   int64_t window_ms,
                   float x,
                   float y) {
    HitResult miss{-1, -1, 0.f};
    if (window_ms <= 0 || chart.drawings.empty()) return miss;

    // Grab-priority: if a drawing is selected, its (visible) handles win first.
    constexpr float kHandleHit = kNodeRingRadius + 6.f;
    if (chart.selected_drawing >= 0 &&
        static_cast<size_t>(chart.selected_drawing) < chart.drawings.size()) {
        const VroomDrawing& d = chart.drawings[chart.selected_drawing];
        const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
        if (std::hypot(x - a.fX, y - a.fY) <= kHandleHit)
            return HitResult{chart.selected_drawing, 0, 0.f};
        if (std::hypot(x - b.fX, y - b.fY) <= kHandleHit)
            return HitResult{chart.selected_drawing, 1, 1.f};
    }

    // Otherwise the nearest line body within tolerance (topmost = last drawn).
    constexpr float kBodyHit = 6.f;
    float best = kBodyHit;
    int32_t best_i = -1;
    float best_t = 0.f;
    for (size_t i = 0; i < chart.drawings.size(); ++i) {
        const VroomDrawing& d = chart.drawings[i];
        const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
        const float dist = dist_to_segment(x, y, a.fX, a.fY, b.fX, b.fY);
        if (dist <= best) {  // <= so later (topmost) drawings win ties
            best = dist;
            best_i = static_cast<int32_t>(i);
            best_t = segment_t(x, y, a.fX, a.fY, b.fX, b.fY);
        }
    }
    return best_i >= 0 ? HitResult{best_i, 2, best_t} : miss;
}

}  // namespace vroom::drawings
