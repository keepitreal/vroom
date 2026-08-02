#include "ma_overlay.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>

#include "viewport.h"

namespace vroom::ma_overlay {

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

    SkPaint line;
    line.setAntiAlias(true);
    line.setColor(static_cast<SkColor>(color));
    // Fade the line in during the candle→line morph (multiplies the color alpha).
    if (opacity < 1.f) {
        line.setAlphaf(line.getAlphaf() * opacity);
    }
    line.setStyle(SkPaint::kStroke_Style);
    line.setStrokeWidth(width > 0.f ? width : 1.5f);

    // Clip to the candle area so a line that runs off-range doesn't bleed into
    // the axis strips or any indicator pane below.
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    canvas->drawPath(path.detach(), line);
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
