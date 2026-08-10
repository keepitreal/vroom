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

#include "gradient.h"
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

// The close-price polyline, walked left to right. Shared by the stroke and the
// gradient fill so both derive from identical geometry, mid-morph included.
//
// Descending slots — position counting back from the right edge, the pairing a
// timeframe switch preserves — which walks the line left to right, so the vertex
// order matches draw()'s at morph_t == 1. Every candle has a close, so there are
// no gaps to break the subpath on.
SkPath build_close_path(const Layout& lay,
                        const PriceBounds& bounds,
                        const ::VroomCandle* visible,
                        std::size_t n,
                        int64_t window_ms,
                        int64_t visible_start_ms,
                        int64_t candle_duration_ms,
                        const CandleSnapshot* from,
                        std::size_t from_n,
                        float morph_t) {
    const std::size_t from_count = vroom::morph_from_count(from, from_n, morph_t);
    const std::size_t slots = std::max(n, from_count);
    if (slots == 0) return SkPath();
    const float area_w = vroom::candle_area_width(lay);

    SkPathBuilder path;
    bool pen_down = false;
    for (std::size_t k = slots; k-- > 0;) {
        const ::VroomCandle* to = (k < n) ? &visible[n - 1 - k] : nullptr;
        const CandleSnapshot* frm = (k < from_count) ? &from[k] : nullptr;

        float x, y;
        if (to) {
            const float tx = vroom::candle_center_x(
                lay, to->time_ms, candle_duration_ms, visible_start_ms, window_ms);
            const float ty = vroom::price_to_y(lay, bounds, to->close);
            if (frm) {
                // The capture is in band fractions, so it lands on the same pixels
                // it occupied pre-switch even though the bounds changed.
                x = lerp(frm->x * area_w, tx, morph_t);
                y = lerp(vroom::y_at_fraction(lay, frm->close), ty, morph_t);
            } else {
                x = tx;
                y = ty;
            }
        } else {
            // A slot only the outgoing data had. Counts match in practice, so this
            // just keeps gappy or short-history data from breaking the line.
            x = frm->x * area_w;
            y = vroom::y_at_fraction(lay, frm->close);
        }

        if (pen_down) {
            path.lineTo(x, y);
        } else {
            path.moveTo(x, y);
            pen_down = true;
        }
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
                     float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const SkPath path = build_close_path(lay, bounds, visible, n, window_ms,
                                        visible_start_ms, candle_duration_ms,
                                        from, from_n, morph_t);
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
                         float morph_t) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    gradient_opacity = std::clamp(gradient_opacity, 0.f, 1.f);
    opacity = std::clamp(opacity, 0.f, 1.f);
    const float alpha = gradient_opacity * opacity;
    if (alpha <= 0.f) return;
    morph_t = std::clamp(morph_t, 0.f, 1.f);

    const SkPath line = build_close_path(lay, bounds, visible, n, window_ms,
                                        visible_start_ms, candle_duration_ms,
                                        from, from_n, morph_t);
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
