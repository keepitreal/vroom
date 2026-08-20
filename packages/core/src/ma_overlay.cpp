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

// Width of the background-colored ring that separates the tip dot from the line
// and from the pulse expanding out behind it.
constexpr float kTipBorderPx = 2.f;

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
          float opacity) {
    if (!canvas || !values_visible || n == 0 || candle_right <= 0.f ||
        candle_area_h <= 0.f) {
        return;
    }
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;

    // SkPathBuilder (not SkPath's edit methods, removed in newer Skia tips).
    SkPathBuilder path;
    bool pen_down = false;
    for (std::size_t i = 0; i < n; ++i) {
        const double v = values_visible[i];
        if (!std::isfinite(v)) {
            pen_down = false;
            continue;
        }
        const float x = vroom::candle_center_x(
            lay, visible[i].time_ms, candle_duration_ms, visible_start_ms,
            window_ms);
        const float y = vroom::price_to_y(lay, bounds, v);
        if (pen_down && !(break_before && break_before[i])) {
            path.lineTo(x, y);
        } else {
            path.moveTo(x, y);
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

    // Scaling off the stroke keeps the marker proportionate at any line width;
    // the floor stops a hairline chart from getting an invisible dot.
    const float w = line_width > 0.f ? line_width : 1.5f;
    const float dot_r = std::max(2.f, w * 1.5f);
    const float border_r = dot_r + kTipBorderPx;

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));

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
                  float opacity) {
    if (!canvas || !upper_visible || !lower_visible || n == 0 ||
        candle_right <= 0.f || candle_area_h <= 0.f) {
        return;
    }
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;

    // One closed contour per maximal run where both series are finite; a
    // single-point run has no area. Multiple runs (e.g. around a data gap)
    // become multiple contours in one path.
    SkPathBuilder path;
    std::size_t i = 0;
    while (i < n) {
        if (!std::isfinite(upper_visible[i]) ||
            !std::isfinite(lower_visible[i])) {
            ++i;
            continue;
        }
        std::size_t e = i;
        while (e + 1 < n && std::isfinite(upper_visible[e + 1]) &&
               std::isfinite(lower_visible[e + 1])) {
            ++e;
        }
        if (e > i) {
            const auto x_at = [&](std::size_t k) {
                return vroom::candle_center_x(lay, visible[k].time_ms,
                                              candle_duration_ms,
                                              visible_start_ms, window_ms);
            };
            path.moveTo(x_at(i), vroom::price_to_y(lay, bounds, upper_visible[i]));
            for (std::size_t k = i + 1; k <= e; ++k) {
                path.lineTo(x_at(k), vroom::price_to_y(lay, bounds, upper_visible[k]));
            }
            for (std::size_t k = e + 1; k-- > i;) {
                path.lineTo(x_at(k), vroom::price_to_y(lay, bounds, lower_visible[k]));
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

}  // namespace vroom::ma_overlay
