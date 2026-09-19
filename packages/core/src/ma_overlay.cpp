#include "ma_overlay.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>
#include <vector>

#include "curve.h"
#include "gradient.h"
#include "line_morph.h"
#include "tip_geometry.h"
#include "tip_pulse.h"
#include "viewport.h"

namespace vroom::ma_overlay {

namespace {
// Strokes a built polyline, clipped to the candle area so a line that runs off
// range doesn't bleed into the axis strips or any indicator pane below.
void stroke_path(SkCanvas* canvas,
                 const SkPath& path,
                 float candle_right,
                 float candle_area_h,
                 uint32_t color,
                 float width,
                 float opacity) {
    SkPaint line;
    line.setAntiAlias(true);
    line.setColor(static_cast<SkColor>(color));
    // Fade the line in during the candle→line morph (multiplies the color alpha).
    if (opacity < 1.f) {
        line.setAlphaf(line.getAlphaf() * opacity);
    }
    line.setStyle(SkPaint::kStroke_Style);
    line.setStrokeWidth(width > 0.f ? width : 1.5f);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    canvas->drawPath(path, line);
    canvas->restore();
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Slot `k` of an indicator series in screen space, blended from the outgoing
// capture toward the new value. Slot 0 is the rightmost vertex on both sides —
// the pairing a timeframe switch preserves, same as close_vertex below.
//
// The capture is in fractions of the candle area and the price band, so it
// lands on the pixels it occupied pre-switch even though both the surface and
// the bounds may have changed under it.
MorphVertex series_vertex(const Layout& lay,
                          const PriceBounds& bounds,
                          const ::VroomCandle* visible,
                          std::size_t n,
                          const double* values,
                          int64_t window_ms,
                          int64_t visible_start_ms,
                          int64_t candle_duration_ms,
                          int64_t time_shift_ms,
                          const LineMorph* from,
                          std::size_t from_count,
                          float morph_t,
                          std::size_t k) {
    MorphVertex to;
    if (values && k < n) {
        const std::size_t i = n - 1 - k;
        const double v = values[i];
        if (std::isfinite(v)) {
            to = MorphVertex{
                vroom::candle_center_x(lay, visible[i].time_ms + time_shift_ms,
                                       candle_duration_ms, visible_start_ms,
                                       window_ms),
                vroom::price_to_y(lay, bounds, v), true};
        }
    }
    MorphVertex frm;
    if (k < from_count) {
        const LineSnapshot& s = from->pts[k];
        if (s.valid) {
            frm = MorphVertex{s.x * vroom::candle_area_width(lay),
                              vroom::y_at_fraction(lay, s.y), true};
        }
    }
    return vroom::morph_vertex(to, frm, morph_t);
}

// The same series walked left to right into `out`, for the two fills — they
// need to look ahead for crossovers and back along the opposite edge, so they
// can't stream the way the stroke does. out[j] is slot slots-1-j.
void resolve_series(const Layout& lay,
                    const PriceBounds& bounds,
                    const ::VroomCandle* visible,
                    std::size_t n,
                    const double* values,
                    int64_t window_ms,
                    int64_t visible_start_ms,
                    int64_t candle_duration_ms,
                    int64_t time_shift_ms,
                    const LineMorph* from,
                    float morph_t,
                    std::size_t slots,
                    std::vector<MorphVertex>& out) {
    const std::size_t from_count = vroom::morph_line_count(from, morph_t);
    out.clear();
    out.reserve(slots);
    for (std::size_t j = 0; j < slots; ++j) {
        out.push_back(series_vertex(lay, bounds, visible, n, values, window_ms,
                                    visible_start_ms, candle_duration_ms,
                                    time_shift_ms, from, from_count, morph_t,
                                    slots - 1 - j));
    }
}

// How many vertices a morphing series spans: the new slice and the capture can
// disagree on length, and every slot either defines has to be walked.
inline std::size_t series_slots(std::size_t n, const LineMorph* from,
                                float morph_t) {
    return std::max(n, vroom::morph_line_count(from, morph_t));
}

// One vertex of the close polyline, in screen space. Slot `k` counts back from
// the right edge (0 = the newest close), the pairing a timeframe switch
// preserves.
//
// Shared by the path builder and the tip marker: the dot has to sit on the exact
// pixel the line ends at, including mid-interval-morph where both coordinates
// are interpolated against the outgoing capture.
SkPoint close_vertex(const Layout& lay,
                     const PriceBounds& bounds,
                     const ::VroomCandle* visible,
                     std::size_t n,
                     int64_t window_ms,
                     int64_t visible_start_ms,
                     int64_t candle_duration_ms,
                     const CandleSnapshot* from,
                     std::size_t from_count,
                     float morph_t,
                     std::size_t k) {
    const ::VroomCandle* to = (k < n) ? &visible[n - 1 - k] : nullptr;
    const CandleSnapshot* frm = (k < from_count) ? &from[k] : nullptr;
    if (to) {
        const float tx = vroom::candle_center_x(
            lay, to->time_ms, candle_duration_ms, visible_start_ms, window_ms);
        const float ty = vroom::price_to_y(lay, bounds, to->close);
        if (!frm) return SkPoint{tx, ty};
        // The capture is in band fractions, so it lands on the same pixels it
        // occupied pre-switch even though the bounds changed.
        const float area_w = vroom::candle_area_width(lay);
        return SkPoint{
            lerp(frm->x * area_w, tx, morph_t),
            lerp(vroom::y_at_fraction(lay, frm->close), ty, morph_t)};
    }
    // A slot only the outgoing data had. Counts match in practice, so this just
    // keeps gappy or short-history data from breaking the line.
    return SkPoint{frm->x * vroom::candle_area_width(lay),
                   vroom::y_at_fraction(lay, frm->close)};
}

// The close-price polyline, walked left to right. Shared by the stroke and the
// gradient fill so both derive from identical geometry, mid-morph included.
//
// Descending slots — position counting back from the right edge, the pairing a
// timeframe switch preserves — which walks the line left to right, so the vertex
// order matches draw()'s at morph_t == 1. Every candle has a close, so there are
// no gaps to break the subpath on.
//
// `tension` above 0 rounds the corners (see curve.h). It smooths the *screen*
// positions rather than the closes, which is what keeps the line glued to the
// candles mid-interval-morph, where those positions are themselves interpolated.
SkPath build_close_path(const Layout& lay,
                        const PriceBounds& bounds,
                        const ::VroomCandle* visible,
                        std::size_t n,
                        int64_t window_ms,
                        int64_t visible_start_ms,
                        int64_t candle_duration_ms,
                        const CandleSnapshot* from,
                        std::size_t from_n,
                        float morph_t,
                        float tension) {
    const std::size_t from_count = vroom::morph_from_count(from, from_n, morph_t);
    const std::size_t slots = std::max(n, from_count);
    if (slots == 0) return SkPath();

    // Vertex `i` of the left-to-right walk, so the smoothing pass below can look
    // ahead without a second copy of the morph interpolation.
    const auto vertex = [&](std::size_t i) -> SkPoint {
        return close_vertex(lay, bounds, visible, n, window_ms, visible_start_ms,
                            candle_duration_ms, from, from_count, morph_t,
                            slots - 1 - i);
    };

    // Straight segments by default, and below three points there's no interior
    // vertex to round. Kept as its own pass so the common case emits exactly the
    // path it always has.
    if (!(tension > 0.f) || slots < 3) {
        SkPathBuilder path;
        for (std::size_t i = 0; i < slots; ++i) {
            const SkPoint p = vertex(i);
            if (i == 0) {
                path.moveTo(p);
            } else {
                path.lineTo(p);
            }
        }
        return path.detach();
    }

    // A tangent needs a neighbor on both sides, so smoothing can't stay
    // streaming. Reused across frames — rendering is single-threaded per chart
    // and the buffer is only live for this call — so it stops allocating after
    // the first frame at a given zoom.
    static thread_local std::vector<SkPoint> pts;
    pts.clear();
    pts.reserve(slots);
    for (std::size_t i = 0; i < slots; ++i) pts.push_back(vertex(i));

    const auto secant_at = [&](std::size_t i) {
        return vroom::curve::secant(pts[i].fX, pts[i].fY,
                                    pts[i + 1].fX, pts[i + 1].fY);
    };
    // Endpoints have only one neighboring secant to lean on; interiors get the
    // monotone-limited blend of both. Recomputing a secant or two beats carrying
    // a second buffer.
    const auto tangent_at = [&](std::size_t i) {
        if (i == 0) return secant_at(0);
        if (i + 1 == pts.size()) return secant_at(pts.size() - 2);
        return vroom::curve::monotone_tangent(secant_at(i - 1), secant_at(i));
    };

    SkPathBuilder path;
    path.moveTo(pts[0]);
    for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
        // Flat or backwards spacing — duplicate timestamps, or the compressed x
        // a morph can pass through — has no meaningful tangent, and curving
        // through it would fold the line back on itself.
        if (!(pts[i + 1].fX > pts[i].fX)) {
            path.lineTo(pts[i + 1]);
            continue;
        }
        const vroom::curve::Controls c = vroom::curve::segment_controls(
            pts[i].fX, pts[i].fY, pts[i + 1].fX, pts[i + 1].fY,
            tangent_at(i), tangent_at(i + 1), tension);
        path.cubicTo(c.c1x, c.c1y, c.c2x, c.c2y, pts[i + 1].fX, pts[i + 1].fY);
    }
    return path.detach();
}
}  // namespace

void draw(SkCanvas* canvas,
          const Layout& lay,
          const PriceBounds& bounds,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* values_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float candle_area_h,
          uint32_t color,
          float width,
          const unsigned char* break_before,
          float opacity,
          int64_t time_shift_ms,
          const LineMorph* from,
          float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const std::size_t from_count = vroom::morph_line_count(from, morph_t);
    const std::size_t slots = std::max(n, from_count);
    if (slots == 0 || (!values_visible && from_count == 0)) return;

    // Descending slots, which walks the line left to right. Settled (no
    // capture) this visits exactly the source indices 0..n-1 in order, so the
    // path is the one it has always emitted.
    // SkPathBuilder (not SkPath's edit methods, removed in newer Skia tips).
    SkPathBuilder path;
    bool pen_down = false;
    for (std::size_t j = 0; j < slots; ++j) {
        const std::size_t k = slots - 1 - j;
        const MorphVertex p = series_vertex(
            lay, bounds, visible, n, values_visible, window_ms,
            visible_start_ms, candle_duration_ms, time_shift_ms, from,
            from_count, morph_t, k);
        if (!p.valid) {
            pen_down = false;
            continue;
        }
        // Session breaks are a property of the incoming data, so a slot the new
        // series doesn't reach can't carry one.
        const bool breaks =
            break_before && k < n && break_before[n - 1 - k] != 0;
        if (pen_down && !breaks) {
            path.lineTo(p.x, p.y);
        } else {
            path.moveTo(p.x, p.y);
            pen_down = true;
        }
    }

    stroke_path(canvas, path.detach(), candle_right, candle_area_h, color, width,
                opacity);
}

void draw_close_line(SkCanvas* canvas,
                     const Layout& lay,
                     const PriceBounds& bounds,
                     const ::VroomCandle* visible,
                     std::size_t n,
                     int64_t window_ms,
                     int64_t visible_start_ms,
                     int64_t candle_duration_ms,
                     float candle_right,
                     float candle_area_h,
                     uint32_t color,
                     float width,
                     float opacity,
                     const CandleSnapshot* from,
                     std::size_t from_n,
                     float morph_t,
                     float tension) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);
    tension = std::clamp(tension, 0.f, 1.f);

    const SkPath path = build_close_path(lay, bounds, visible, n, window_ms,
                                        visible_start_ms, candle_duration_ms,
                                        from, from_n, morph_t, tension);
    if (path.isEmpty()) return;

    stroke_path(canvas, path, candle_right, candle_area_h, color, width, opacity);
}

void draw_close_gradient(SkCanvas* canvas,
                         const Layout& lay,
                         const PriceBounds& bounds,
                         const ::VroomCandle* visible,
                         std::size_t n,
                         int64_t window_ms,
                         int64_t visible_start_ms,
                         int64_t candle_duration_ms,
                         float candle_right,
                         float candle_area_h,
                         uint32_t color,
                         float gradient_opacity,
                         float opacity,
                         const CandleSnapshot* from,
                         std::size_t from_n,
                         float morph_t,
                         float tension) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    gradient_opacity = std::clamp(gradient_opacity, 0.f, 1.f);
    opacity = std::clamp(opacity, 0.f, 1.f);
    const float alpha = gradient_opacity * opacity;
    if (alpha <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);
    tension = std::clamp(tension, 0.f, 1.f);

    const SkPath line = build_close_path(lay, bounds, visible, n, window_ms,
                                        visible_start_ms, candle_duration_ms,
                                        from, from_n, morph_t, tension);
    if (line.isEmpty()) return;

    // The polyline runs strictly left to right, so its bounds give the first and
    // last vertex x plus the peak the ramp starts from. A peak above the pane
    // (line zoomed off the top) would stretch the ramp, so anchor at the pane.
    const SkRect b = line.getBounds();
    const float top = std::max(b.top(), 0.f);
    if (top >= candle_area_h) return;

    // Drop from the last vertex to the pane bottom and back along it, turning the
    // polyline into the closed region beneath the line.
    SkPathBuilder area(line);
    area.lineTo(b.right(), candle_area_h);
    area.lineTo(b.left(), candle_area_h);
    area.close();

    const SkPoint pts[2] = {SkPoint::Make(0.f, top),
                            SkPoint::Make(0.f, candle_area_h)};
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    fill.setShader(
        vroom::linear_alpha_ramp(pts, static_cast<SkColor>(color), alpha, 0.f));

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    canvas->drawPath(area.detach(), fill);
    canvas->restore();
}

void draw_close_tip(SkCanvas* canvas,
                    const Layout& lay,
                    const PriceBounds& bounds,
                    const ::VroomCandle* visible,
                    std::size_t n,
                    int64_t window_ms,
                    int64_t visible_start_ms,
                    int64_t candle_duration_ms,
                    float candle_right,
                    float clip_right,
                    float candle_area_h,
                    uint32_t line_color,
                    uint32_t bg_color,
                    float line_width,
                    float opacity,
                    bool pulse,
                    float pulse_phase,
                    const CandleSnapshot* from,
                    std::size_t from_n,
                    float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const std::size_t from_count = vroom::morph_from_count(from, from_n, morph_t);
    const std::size_t slots = std::max(n, from_count);
    if (slots == 0) return;

    const SkPoint tip =
        close_vertex(lay, bounds, visible, n, window_ms, visible_start_ms,
                     candle_duration_ms, from, from_count, morph_t, 0);
    // Panned or scaled out of the pane: the line's end isn't on screen, so
    // there's nothing to mark. The clip below would hide it anyway, but the ring
    // is wide enough that a tip just off-pane would still bleed in.
    if (tip.fY < 0.f || tip.fY > candle_area_h) return;
    if (tip.fX < 0.f || tip.fX > candle_right) return;

    const auto geo = vroom::tip_geometry::of(line_width);
    const float dot_r = geo.dot_r;
    const float border_r = geo.border_r;

    canvas->save();
    // Wider than the on-pane test above: the dot is anchored at the newest
    // candle's center, which on a view pinned to the latest bar is closer to
    // candle_right than the dot's own radius, so it has to reach into the gutter
    // to draw in full. `clip_right` is the gutter's far side — the y-axis
    // strip's edge — which the layout keeps wide enough for the dot (see
    // tip_geometry::gutter_px). The pulse ring is much wider still and clips
    // there; that is intended.
    canvas->clipRect(
        SkRect::MakeLTRB(0.f, 0.f, std::max(candle_right, clip_right),
                         candle_area_h));

    // Ring first: the border paints over its inner edge, so it reads as
    // expanding out from underneath the dot rather than around it.
    if (pulse) {
        const vroom::tip_pulse::Frame f = vroom::tip_pulse::at(pulse_phase);
        const float r = border_r * f.radius_mul;
        if (f.fill_alpha > 0.f) {
            SkPaint fill;
            fill.setAntiAlias(true);
            fill.setColor(static_cast<SkColor>(line_color));
            fill.setAlphaf(fill.getAlphaf() * f.fill_alpha * opacity);
            canvas->drawCircle(tip.fX, tip.fY, r, fill);
        }
        if (f.stroke_alpha > 0.f) {
            SkPaint edge;
            edge.setAntiAlias(true);
            edge.setColor(static_cast<SkColor>(line_color));
            edge.setAlphaf(edge.getAlphaf() * f.stroke_alpha * opacity);
            edge.setStyle(SkPaint::kStroke_Style);
            edge.setStrokeWidth(1.f);
            canvas->drawCircle(tip.fX, tip.fY, r, edge);
        }
    }

    // Filled rather than stroked: an annulus stroked around the dot leaves an
    // antialiased seam where the two meet, which shows up as a dark hairline.
    SkPaint border;
    border.setAntiAlias(true);
    border.setColor(static_cast<SkColor>(bg_color));
    border.setAlphaf(border.getAlphaf() * opacity);
    canvas->drawCircle(tip.fX, tip.fY, border_r, border);

    SkPaint dot;
    dot.setAntiAlias(true);
    dot.setColor(static_cast<SkColor>(line_color));
    dot.setAlphaf(dot.getAlphaf() * opacity);
    canvas->drawCircle(tip.fX, tip.fY, dot_r, dot);

    canvas->restore();
}

void fill_between(SkCanvas* canvas,
                  const Layout& lay,
                  const PriceBounds& bounds,
                  const ::VroomCandle* visible,
                  std::size_t n,
                  const double* upper_visible,
                  const double* lower_visible,
                  int64_t window_ms,
                  int64_t visible_start_ms,
                  int64_t candle_duration_ms,
                  float candle_right,
                  float candle_area_h,
                  uint32_t color,
                  float opacity,
                  const LineMorph* upper_from,
                  const LineMorph* lower_from,
                  float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const std::size_t slots =
        std::max(series_slots(n, upper_from, morph_t),
                 series_slots(n, lower_from, morph_t));
    if (slots == 0) return;

    // Both edges resolved left to right first: mid-morph a vertex is a blend of
    // the captured shape and the new one, and the fill has to be stitched from
    // the same positions the two lines are stroked at.
    static thread_local std::vector<MorphVertex> upper;
    static thread_local std::vector<MorphVertex> lower;
    resolve_series(lay, bounds, visible, n, upper_visible, window_ms,
                   visible_start_ms, candle_duration_ms, 0, upper_from, morph_t,
                   slots, upper);
    resolve_series(lay, bounds, visible, n, lower_visible, window_ms,
                   visible_start_ms, candle_duration_ms, 0, lower_from, morph_t,
                   slots, lower);

    // One closed contour per maximal run where both edges are defined; a
    // single-point run has no area. Multiple runs (e.g. around a data gap)
    // become multiple contours in one path.
    SkPathBuilder path;
    std::size_t i = 0;
    while (i < slots) {
        if (!upper[i].valid || !lower[i].valid) {
            ++i;
            continue;
        }
        std::size_t e = i;
        while (e + 1 < slots && upper[e + 1].valid && lower[e + 1].valid) ++e;
        if (e > i) {
            path.moveTo(upper[i].x, upper[i].y);
            for (std::size_t k = i + 1; k <= e; ++k) {
                path.lineTo(upper[k].x, upper[k].y);
            }
            for (std::size_t k = e + 1; k-- > i;) {
                path.lineTo(lower[k].x, lower[k].y);
            }
            path.close();
        }
        i = e + 1;
    }

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(static_cast<SkColor>(color));
    fill.setAlphaf(fill.getAlphaf() * opacity);
    fill.setStyle(SkPaint::kFill_Style);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    canvas->drawPath(path.detach(), fill);
    canvas->restore();
}

void fill_cloud(SkCanvas* canvas,
                const Layout& lay,
                const PriceBounds& bounds,
                const ::VroomCandle* visible,
                std::size_t n,
                const double* a_visible,
                const double* b_visible,
                int64_t window_ms,
                int64_t visible_start_ms,
                int64_t candle_duration_ms,
                float candle_right,
                float candle_area_h,
                uint32_t above_color,
                uint32_t below_color,
                float opacity,
                int64_t time_shift_ms,
                const LineMorph* a_from,
                const LineMorph* b_from,
                float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const std::size_t slots = std::max(series_slots(n, a_from, morph_t),
                                       series_slots(n, b_from, morph_t));
    if (slots == 0) return;

    static thread_local std::vector<MorphVertex> edge_a;
    static thread_local std::vector<MorphVertex> edge_b;
    resolve_series(lay, bounds, visible, n, a_visible, window_ms,
                   visible_start_ms, candle_duration_ms, time_shift_ms, a_from,
                   morph_t, slots, edge_a);
    resolve_series(lay, bounds, visible, n, b_visible, window_ms,
                   visible_start_ms, candle_duration_ms, time_shift_ms, b_from,
                   morph_t, slots, edge_b);

    // A vertex of the cloud: one x with the two edge heights there. At a
    // crossover the two collapse onto the same y, which is what lets the
    // neighboring contours meet on a point instead of overlapping.
    struct Vertex {
        float x, ya, yb;
    };

    SkPathBuilder above_path;
    SkPathBuilder below_path;
    std::vector<Vertex> contour;

    // Closes the accumulated vertices into the path for whichever side is on
    // top: forward along edge a, back along edge b.
    const auto flush = [&](int sign) {
        if (contour.size() >= 2) {
            SkPathBuilder& into = sign >= 0 ? above_path : below_path;
            into.moveTo(contour[0].x, contour[0].ya);
            for (std::size_t k = 1; k < contour.size(); ++k) {
                into.lineTo(contour[k].x, contour[k].ya);
            }
            for (std::size_t k = contour.size(); k-- > 0;) {
                into.lineTo(contour[k].x, contour[k].yb);
            }
            into.close();
        }
        contour.clear();
    };

    // Which tone a run takes, read off the drawn geometry rather than the raw
    // values: y grows downward, so a above b means ya is the smaller. Mid-morph
    // the values belong to two different resolutions and only the interpolated
    // positions say where the edges actually sit.
    const auto separation = [&](std::size_t k) {
        return static_cast<double>(edge_b[k].y - edge_a[k].y);
    };

    std::size_t i = 0;
    while (i < slots) {
        if (!edge_a[i].valid || !edge_b[i].valid) {
            ++i;
            continue;
        }
        std::size_t e = i;
        while (e + 1 < slots && edge_a[e + 1].valid && edge_b[e + 1].valid) ++e;

        int sign = 0;
        Vertex prev{};
        for (std::size_t k = i; k <= e; ++k) {
            const Vertex v = Vertex{edge_a[k].x, edge_a[k].y, edge_b[k].y};
            const double d = separation(k);
            const int s = d > 0.0 ? 1 : (d < 0.0 ? -1 : 0);
            if (sign != 0 && s != 0 && s != sign) {
                // The spans swapped between the previous bar and this one. Meet
                // them at the crossing so both tones end on the same point.
                const double d0 = separation(k - 1);
                const float t = static_cast<float>(d0 / (d0 - d));
                const float xc = prev.x + (v.x - prev.x) * t;
                const float yc = prev.ya + (v.ya - prev.ya) * t;
                contour.push_back(Vertex{xc, yc, yc});
                flush(sign);
                contour.push_back(Vertex{xc, yc, yc});
                sign = s;
            } else if (sign == 0) {
                sign = s;
            }
            contour.push_back(v);
            prev = v;
        }
        flush(sign);
        i = e + 1;
    }

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    fill.setColor(static_cast<SkColor>(above_color));
    fill.setAlphaf(fill.getAlphaf() * opacity);
    canvas->drawPath(above_path.detach(), fill);
    fill.setColor(static_cast<SkColor>(below_color));
    fill.setAlphaf(fill.getAlphaf() * opacity);
    canvas->drawPath(below_path.detach(), fill);
    canvas->restore();
}

}  // namespace vroom::ma_overlay
