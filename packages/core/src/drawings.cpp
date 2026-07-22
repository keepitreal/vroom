#include "drawings.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <array>
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

// The four corners of the box with opposite corners `a` and `b`, in the fixed
// order used by hit-test `part` indices: a, (b.x,a.y), b, (a.x,b.y). Corner k's
// diagonal is corner (k+2)%4.
std::array<SkPoint, 4> box_corners(SkPoint a, SkPoint b) {
    return {SkPoint{a.fX, a.fY}, SkPoint{b.fX, a.fY}, SkPoint{b.fX, b.fY},
            SkPoint{a.fX, b.fY}};
}

// The normalized (L,T,R,B) rectangle spanning opposite corners `a` and `b`.
SkRect box_rect(SkPoint a, SkPoint b) {
    return SkRect::MakeLTRB(std::min(a.fX, b.fX), std::min(a.fY, b.fY),
                            std::max(a.fX, b.fX), std::max(a.fY, b.fY));
}

// Strokes the box outline and paints a faint fill (~10% of the border alpha) of
// the same color, spanning opposite corners `a` and `b`.
void draw_box(SkCanvas* canvas, SkPoint a, SkPoint b, SkColor color, float width) {
    const SkRect r = box_rect(a, b);

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    // Faint fill: 10% of the border's alpha, same RGB.
    const U8CPU fill_alpha = static_cast<U8CPU>(SkColorGetA(color) * 0.1f);
    fill.setColor(SkColorSetA(color, fill_alpha));
    canvas->drawRect(r, fill);

    SkPaint border;
    border.setAntiAlias(true);
    border.setColor(color);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(width > 0.f ? width : 2.f);
    canvas->drawRect(r, border);
}

// Strokes a freehand path through `pts` (data space). The polyline is smoothed
// with quadratic curves through segment midpoints — each captured point becomes
// a control point, so the stroke reads as one organic curve instead of a chain
// of visible corners. Round caps/joins give it a pen-like feel.
void draw_path(SkCanvas* canvas, const VroomChart& chart, const Layout& lay,
               const PriceBounds& bounds, int64_t window_ms,
               const std::vector<VroomDrawPoint>& pts, SkColor color,
               float width) {
    if (pts.empty()) return;
    const float w = width > 0.f ? width : 2.f;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);

    // A stroke that's still a single sample renders as a dot, so pressing down
    // gives immediate feedback before the pointer moves.
    if (pts.size() == 1) {
        const SkPoint p = to_px(chart, lay, bounds, window_ms, pts[0]);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(p.fX, p.fY, w * 0.5f, paint);
        return;
    }

    SkPathBuilder builder;
    builder.moveTo(to_px(chart, lay, bounds, window_ms, pts[0]));
    // Curve through every interior point, ending each quad at the midpoint of
    // the next segment; finish with a straight run to the final sample.
    for (size_t i = 1; i + 1 < pts.size(); ++i) {
        const SkPoint cur = to_px(chart, lay, bounds, window_ms, pts[i]);
        const SkPoint next = to_px(chart, lay, bounds, window_ms, pts[i + 1]);
        builder.quadTo(cur, SkPoint{(cur.fX + next.fX) * 0.5f,
                                    (cur.fY + next.fY) * 0.5f});
    }
    builder.lineTo(to_px(chart, lay, bounds, window_ms, pts.back()));

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(w);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawPath(builder.detach(), paint);
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

// True when (px,py) is inside the box (the grabbable fill area).
bool point_in_rect(float px, float py, SkPoint a, SkPoint b) {
    const SkRect r = box_rect(a, b);
    return px >= r.fLeft && px <= r.fRight && py >= r.fTop && py <= r.fBottom;
}

// Shortest distance from (px,py) to the freehand polyline through `pts`, in px.
// Cheap bounding-box reject first (a stroke can hold many points and most
// hit-tests miss), then distance-to-segment across consecutive samples — the
// same two-stage approach Excalidraw uses for its freedraw elements. Returns a
// value greater than `tol` on a clear miss.
float dist_to_path(float px, float py, const VroomChart& chart, const Layout& lay,
                   const PriceBounds& bounds, int64_t window_ms,
                   const std::vector<VroomDrawPoint>& pts, float tol) {
    const float kMiss = tol + 1.f;
    if (pts.size() < 2) return kMiss;

    float min_x = 0.f, max_x = 0.f, min_y = 0.f, max_y = 0.f;
    for (size_t i = 0; i < pts.size(); ++i) {
        const SkPoint p = to_px(chart, lay, bounds, window_ms, pts[i]);
        if (i == 0) {
            min_x = max_x = p.fX;
            min_y = max_y = p.fY;
        } else {
            min_x = std::min(min_x, p.fX);
            max_x = std::max(max_x, p.fX);
            min_y = std::min(min_y, p.fY);
            max_y = std::max(max_y, p.fY);
        }
    }
    if (px < min_x - tol || px > max_x + tol || py < min_y - tol ||
        py > max_y + tol) {
        return kMiss;
    }

    float best = kMiss;
    SkPoint prev = to_px(chart, lay, bounds, window_ms, pts[0]);
    for (size_t i = 1; i < pts.size(); ++i) {
        const SkPoint cur = to_px(chart, lay, bounds, window_ms, pts[i]);
        best = std::min(best,
                        dist_to_segment(px, py, prev.fX, prev.fY, cur.fX, cur.fY));
        prev = cur;
    }
    return best;
}

// Shortest distance from (px,py) to the box's four edges, in px (0 when on an
// edge). Used so the border is grabbable even outside the fill's tolerance.
float dist_to_rect_edges(float px, float py, SkPoint a, SkPoint b) {
    const auto c = box_corners(a, b);
    float best = dist_to_segment(px, py, c[0].fX, c[0].fY, c[1].fX, c[1].fY);
    for (int i = 1; i < 4; ++i) {
        const SkPoint& p0 = c[i];
        const SkPoint& p1 = c[(i + 1) % 4];
        best = std::min(best,
                        dist_to_segment(px, py, p0.fX, p0.fY, p1.fX, p1.fY));
    }
    return best;
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
        for (const auto& d : chart.drawings) {
            if (d.kind == 2) {
                draw_path(canvas, chart, lay, bounds, window_ms, d.points,
                          static_cast<SkColor>(d.color), d.width);
                continue;
            }
            const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
            const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
            if (d.kind == 1) {
                draw_box(canvas, a, b, static_cast<SkColor>(d.color), d.width);
                continue;
            }
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
        const auto& d = chart.drawings[chart.selected_drawing];
        const SkPoint sa = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint sb = to_px(chart, lay, bounds, window_ms, d.b);
        if (d.kind == 2) {
            // Pencil: anchors on the first/last point are a visual cue that the
            // stroke is movable — they aren't grab handles, so never enlarged.
            draw_node(canvas, sa);
            draw_node(canvas, sb);
        } else if (d.kind == 1) {
            // Four corner handles. The gesture layer stores the grabbed corner
            // as endpoint `a` (its diagonal as `b`), so corner 0 is the active
            // one while dragging.
            const auto corners = box_corners(sa, sb);
            for (int i = 0; i < 4; ++i) {
                draw_node(canvas, corners[i],
                          (chart.grabbed_endpoint == 0 && i == 0) ? 1.5f : 1.f);
            }
        } else {
            draw_node(canvas, sa, chart.grabbed_endpoint == 0 ? 1.5f : 1.f);
            draw_node(canvas, sb, chart.grabbed_endpoint == 1 ? 1.5f : 1.f);
        }
    }

    if (!chart.draft_active) return;

    // 1c. Freehand stroke in progress: draw the live path, clipped like the
    //     committed shapes. It grows a point at a time via append_draft_point.
    if (chart.draft_kind == 2) {
        if (chart.draft_points.empty()) return;
        canvas->save();
        canvas->clipRect(clip);
        draw_path(canvas, chart, lay, bounds, window_ms, chart.draft_points,
                  static_cast<SkColor>(chart.draft_color), chart.draft_width);
        canvas->restore();
        return;
    }

    const SkPoint a = to_px(chart, lay, bounds, window_ms, chart.draft_a);
    const bool has_b = chart.draft_has_b;
    const SkPoint b =
        has_b ? to_px(chart, lay, bounds, window_ms, chart.draft_b) : a;

    // 2. Live preview (A->B), clipped to the candle area. Only while the second
    //    point is still being placed (draft_guide); once committed, the solid
    //    shape comes from chart.drawings instead. A box previews as a rectangle;
    //    a line as a guideline.
    if (chart.draft_guide && has_b) {
        canvas->save();
        canvas->clipRect(clip);
        if (chart.draft_kind == 1) {
            draw_box(canvas, a, b, static_cast<SkColor>(chart.draft_color),
                     chart.draft_width);
        } else {
            SkPaint guide;
            guide.setAntiAlias(true);
            guide.setColor(static_cast<SkColor>(chart.draft_color));
            guide.setStyle(SkPaint::kStroke_Style);
            guide.setStrokeWidth(chart.draft_width > 0.f ? chart.draft_width : 2.f);
            canvas->drawLine(a, b, guide);
        }
        canvas->restore();
    }

    // 3. Node dots on top (not clipped, so an edge dot still renders fully).
    //    While guiding (placing the second point) only the anchor dot shows; the
    //    moving end is conveyed by the preview shape. Once selected (committed)
    //    both endpoints show dots.
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
        const auto& d = chart.drawings[chart.selected_drawing];
        const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
        if (d.kind == 2) {
            // Pencil has no grab handles — its anchors translate like any other
            // part of the stroke, so fall through to the body pass.
        } else if (d.kind == 1) {
            // Box: four corner handles (part 0..3).
            const auto corners = box_corners(a, b);
            for (int c = 0; c < 4; ++c) {
                if (std::hypot(x - corners[c].fX, y - corners[c].fY) <= kHandleHit)
                    return HitResult{chart.selected_drawing, c, 0.f};
            }
        } else {
            if (std::hypot(x - a.fX, y - a.fY) <= kHandleHit)
                return HitResult{chart.selected_drawing, 0, 0.f};
            if (std::hypot(x - b.fX, y - b.fY) <= kHandleHit)
                return HitResult{chart.selected_drawing, 1, 1.f};
        }
    }

    // Otherwise the nearest drawing body within tolerance (topmost = last drawn).
    // Line body is distance-to-segment (part 2); box body is the interior (dist
    // 0) or a nearby edge (part 4).
    constexpr float kBodyHit = 6.f;
    float best = kBodyHit;
    int32_t best_i = -1;
    int32_t best_part = 2;
    float best_t = 0.f;
    for (size_t i = 0; i < chart.drawings.size(); ++i) {
        const auto& d = chart.drawings[i];
        if (d.kind == 2) {
            const float dist = dist_to_path(x, y, chart, lay, bounds, window_ms,
                                            d.points, kBodyHit);
            if (dist <= best) {  // <= so later (topmost) drawings win ties
                best = dist;
                best_i = static_cast<int32_t>(i);
                best_part = 5;
                best_t = 0.f;
            }
            continue;
        }
        const SkPoint a = to_px(chart, lay, bounds, window_ms, d.a);
        const SkPoint b = to_px(chart, lay, bounds, window_ms, d.b);
        if (d.kind == 1) {
            const float dist = point_in_rect(x, y, a, b)
                                   ? 0.f
                                   : dist_to_rect_edges(x, y, a, b);
            if (dist <= best) {  // <= so later (topmost) drawings win ties
                best = dist;
                best_i = static_cast<int32_t>(i);
                best_part = 4;
                best_t = 0.f;
            }
        } else {
            const float dist = dist_to_segment(x, y, a.fX, a.fY, b.fX, b.fY);
            if (dist <= best) {  // <= so later (topmost) drawings win ties
                best = dist;
                best_i = static_cast<int32_t>(i);
                best_part = 2;
                best_t = segment_t(x, y, a.fX, a.fY, b.fX, b.fY);
            }
        }
    }
    return best_i >= 0 ? HitResult{best_i, best_part, best_t} : miss;
}

}  // namespace vroom::drawings
