#include "candles.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#pragma clang diagnostic pop

#include <algorithm>

#include "theme.h"
#include "viewport.h"

namespace vroom::candles {

void draw(SkCanvas* canvas,
          const ::VroomCandle* visible,
          std::size_t n,
          const Layout& lay,
          const Theme& theme,
          const PriceBounds& bounds,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms) {
    if (!canvas || n == 0) return;

    const float body_w = vroom::candle_body_width(
        lay, window_ms, candle_duration_ms);

    SkPaint bull_paint;
    bull_paint.setAntiAlias(true);
    bull_paint.setColor(theme.colors[VROOM_COLOR_BULL]);

    SkPaint bear_paint;
    bear_paint.setAntiAlias(true);
    bear_paint.setColor(theme.colors[VROOM_COLOR_BEAR]);

    SkPaint wick_bull;
    wick_bull.setAntiAlias(true);
    wick_bull.setColor(theme.colors[VROOM_COLOR_BULL]);
    wick_bull.setStrokeWidth(theme.floats[VROOM_FLOAT_WICK_WIDTH_PX]);
    wick_bull.setStyle(SkPaint::kStroke_Style);

    SkPaint wick_bear = wick_bull;
    wick_bear.setColor(theme.colors[VROOM_COLOR_BEAR]);

    const float half_body = body_w * 0.5f;

    for (std::size_t i = 0; i < n; ++i) {
        const auto& c = visible[i];
        const bool bull = c.close >= c.open;

        const float cx = vroom::candle_center_x(
            lay, c.time_ms, candle_duration_ms,
            visible_start_ms, window_ms);
        const float y_high = vroom::price_to_y(lay, bounds, c.high);
        const float y_low = vroom::price_to_y(lay, bounds, c.low);
        const float y_open = vroom::price_to_y(lay, bounds, c.open);
        const float y_close = vroom::price_to_y(lay, bounds, c.close);

        canvas->drawLine(cx, y_high, cx, y_low,
                         bull ? wick_bull : wick_bear);

        const float y_top = std::min(y_open, y_close);
        const float y_bot = std::max(y_open, y_close);
        const float h = std::max(1.f, y_bot - y_top);
        canvas->drawRect(
            SkRect::MakeXYWH(cx - half_body, y_top, body_w, h),
            bull ? bull_paint : bear_paint);
    }
}

}  // namespace vroom::candles
