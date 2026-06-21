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
          const unsigned char* break_before) {
    if (!canvas || !values_visible || n == 0 || candle_right <= 0.f ||
        candle_area_h <= 0.f) {
        return;
    }

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
    line.setStyle(SkPaint::kStroke_Style);
    line.setStrokeWidth(width > 0.f ? width : 1.5f);

    // Clip to the candle area so a line that runs off-range doesn't bleed into
    // the axis strips or any indicator pane below.
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    canvas->drawPath(path.detach(), line);
    canvas->restore();
}

}  // namespace vroom::ma_overlay
