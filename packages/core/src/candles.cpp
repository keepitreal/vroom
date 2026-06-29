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

namespace {
// New border/wick colors use a transparent sentinel (alpha == 0) to mean
// "inherit the body fill color". Resolve to `fill` in that case.
inline uint32_t resolve_color(uint32_t color, uint32_t fill) {
    return (color >> 24) == 0 ? fill : color;
}
}  // namespace

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

    const uint32_t fill_bull = theme.colors[VROOM_COLOR_BULL];
    const uint32_t fill_bear = theme.colors[VROOM_COLOR_BEAR];

    SkPaint bull_paint;
    bull_paint.setAntiAlias(true);
    bull_paint.setColor(fill_bull);

    SkPaint bear_paint;
    bear_paint.setAntiAlias(true);
    bear_paint.setColor(fill_bear);

    SkPaint wick_bull;
    wick_bull.setAntiAlias(true);
    wick_bull.setColor(
        resolve_color(theme.colors[VROOM_COLOR_WICK_BULL], fill_bull));
    wick_bull.setStrokeWidth(theme.floats[VROOM_FLOAT_WICK_WIDTH_PX]);
    wick_bull.setStyle(SkPaint::kStroke_Style);

    SkPaint wick_bear = wick_bull;
    wick_bear.setColor(
        resolve_color(theme.colors[VROOM_COLOR_WICK_BEAR], fill_bear));

    // 1px body outlines. Default sentinel resolves to the fill color, so the
    // border is invisible until a consumer sets borderBull/borderBear.
    SkPaint border_bull;
    border_bull.setAntiAlias(true);
    border_bull.setStyle(SkPaint::kStroke_Style);
    border_bull.setStrokeWidth(1.f);
    border_bull.setColor(
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BULL], fill_bull));

    SkPaint border_bear = border_bull;
    border_bear.setColor(
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BEAR], fill_bear));

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
        const SkRect body = SkRect::MakeXYWH(cx - half_body, y_top, body_w, h);
        canvas->drawRect(body, bull ? bull_paint : bear_paint);
        canvas->drawRect(body, bull ? border_bull : border_bear);
    }
}

}  // namespace vroom::candles
