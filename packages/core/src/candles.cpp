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
          int64_t candle_duration_ms,
          float collapse,
          float opacity) {
    if (!canvas || n == 0) return;

    collapse = std::clamp(collapse, 0.f, 1.f);
    opacity = std::clamp(opacity, 0.f, 1.f);
    if (opacity <= 0.f) return;

    // During the candle→line morph, fade the entire candle layer as one unit so
    // overlapping bars/wicks composite cleanly instead of stacking alpha.
    const bool fade_layer = opacity < 0.999f;
    if (fade_layer) {
        canvas->saveLayerAlpha(nullptr,
                               static_cast<U8CPU>(opacity * 255.f + 0.5f));
    }

    const float full_body_w = vroom::candle_body_width(
        lay, window_ms, candle_duration_ms);
    // Thin the body toward the line stroke width as candles collapse to the line.
    const float line_w = theme.floats[VROOM_FLOAT_LINE_WIDTH_PX];
    const float body_w = full_body_w + (line_w - full_body_w) * collapse;

    const uint32_t fill_bull = theme.colors[VROOM_COLOR_BULL];
    const uint32_t fill_bear = theme.colors[VROOM_COLOR_BEAR];

    const float body_r = theme.floats[VROOM_FLOAT_CANDLE_RADIUS_PX];
    const bool wick_round = theme.floats[VROOM_FLOAT_WICK_ROUND_CAP] > 0.5f;

    constexpr float kBorderWidthPx = 1.f;
    // Draw the border only when it differs from the fill; an inherited/transparent
    // border color (the default, and the "hide" path) renders nothing — so a hidden
    // border costs no pixels and never affects candle width.
    const bool draw_border_bull =
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BULL], fill_bull) != fill_bull;
    const bool draw_border_bear =
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BEAR], fill_bear) != fill_bear;

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
    wick_bull.setStrokeCap(wick_round ? SkPaint::kRound_Cap : SkPaint::kButt_Cap);

    SkPaint wick_bear = wick_bull;
    wick_bear.setColor(
        resolve_color(theme.colors[VROOM_COLOR_WICK_BEAR], fill_bear));

    // 1px body outlines, drawn *inside* the body (see the inset at draw time) so
    // a contrasting border never widens the candle. Default sentinel resolves to
    // the fill color; such borders are skipped entirely (draw_border_* above).
    SkPaint border_bull;
    border_bull.setAntiAlias(true);
    border_bull.setStyle(SkPaint::kStroke_Style);
    border_bull.setStrokeWidth(kBorderWidthPx);
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
        const float y_close = vroom::price_to_y(lay, bounds, c.close);
        // Collapse high/low/open toward the close so the candle folds into its
        // close point (the line vertex) as `collapse` → 1.
        const float y_high =
            vroom::price_to_y(lay, bounds, c.high) * (1.f - collapse) +
            y_close * collapse;
        const float y_low =
            vroom::price_to_y(lay, bounds, c.low) * (1.f - collapse) +
            y_close * collapse;
        const float y_open =
            vroom::price_to_y(lay, bounds, c.open) * (1.f - collapse) +
            y_close * collapse;

        canvas->drawLine(cx, y_high, cx, y_low,
                         bull ? wick_bull : wick_bear);

        const float y_top = std::min(y_open, y_close);
        const float y_bot = std::max(y_open, y_close);
        const float h = std::max(1.f, y_bot - y_top);
        const SkRect body = SkRect::MakeXYWH(cx - half_body, y_top, body_w, h);
        // Corner radius clamped so thin candles / short bodies don't over-round.
        const float r = std::min({body_r, body_w * 0.5f, h * 0.5f});
        const SkPaint& fill = bull ? bull_paint : bear_paint;
        if (r > 0.f) {
            canvas->drawRoundRect(body, r, r, fill);
        } else {
            canvas->drawRect(body, fill);
        }

        const bool draw_border = bull ? draw_border_bull : draw_border_bear;
        // Inset by half the stroke so the centered stroke falls fully within the
        // body: the border overlaps the fill's outer edge instead of straddling
        // it, keeping the candle's footprint exactly body_w. Skip when the body is
        // too small to hold it.
        if (draw_border && body_w > kBorderWidthPx && h > kBorderWidthPx) {
            const SkRect inner =
                body.makeInset(kBorderWidthPx * 0.5f, kBorderWidthPx * 0.5f);
            const SkPaint& bp = bull ? border_bull : border_bear;
            // Concentric radius so the inside border follows the rounded body.
            const float ir = std::max(0.f, r - kBorderWidthPx * 0.5f);
            if (ir > 0.f) {
                canvas->drawRoundRect(inner, ir, ir, bp);
            } else {
                canvas->drawRect(inner, bp);
            }
        }
    }

    if (fade_layer) canvas->restore();
}

}  // namespace vroom::candles
