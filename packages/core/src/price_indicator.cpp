#include "price_indicator.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include <cstdio>
#include <cstring>

#include "chart.h"
#include "fonts.h"
#include "price_format.h"
#include "theme.h"
#include "ticks.h"
#include "viewport.h"

namespace vroom::price_indicator {

namespace {
constexpr SkScalar kDash[2] = {2.f, 2.f};
constexpr float kPadV = 4.f;     // box padding above/below the text
constexpr float kPadH = 8.f;     // box padding left/right of the text
constexpr float kCorner = 6.f;   // rounded-box corner radius
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h) {
    if (!canvas || chart.candles.empty()) return;

    // The "current price" is the latest period's close, regardless of whether
    // that candle is horizontally in view.
    const ::VroomCandle& last = chart.candles.back();
    const bool bull = last.close >= last.open;
    const SkColor color =
        chart.theme.colors[bull ? VROOM_COLOR_ACCENT_BULL : VROOM_COLOR_ACCENT_BEAR];

    const float y = vroom::price_to_y(lay, bounds, last.close);
    if (y < 0.f || y > candle_area_h) return;  // price scrolled off-range

    // Dotted line from the left edge to the y-axis separator.
    SkPaint line;
    line.setAntiAlias(true);
    line.setColor(color);
    line.setStrokeWidth(1.f);
    line.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(0.f, y, candle_right, y, line);

    // Price box in the y-axis strip. Needs the axis typeface; if it isn't
    // loaded yet, the line alone still conveys the level. Same when the y-axis
    // is hidden — the level stays readable, it just loses its badge.
    auto tf = vroom::axis_typeface();
    if (!tf) return;
    if (lay.y_axis_opacity <= 0.f) return;

    SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    char buf[48];
    const vroom::PriceFormat fmt = vroom::with_tick_guard(
        chart.price_fmt,
        vroom::pick_price_interval(bounds.max - bounds.min, candle_area_h));
    vroom::format_price(buf, sizeof(buf), last.close, fmt);
    const size_t len = std::strlen(buf);

    // Measure the tight glyph bounds (origin at the baseline) so we can center
    // the actual rendered digits on `y` — robust to fonts whose cap-height
    // metric doesn't match the digit extent.
    SkRect tb;
    const float text_w = font.measureText(buf, len, SkTextEncoding::kUTF8, &tb);
    const float glyph_h = tb.height();

    // Box wraps the digits (width follows the text), centered on the y-axis
    // container's center so its text shares the same column as the y-axis
    // labels, and vertically centered on the price y.
    const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
    const float box_h = glyph_h + 2.f * kPadV;
    const float box_w = text_w + 2.f * kPadH;
    const float box_left = axis_center_x - box_w * 0.5f;
    const float top = y - box_h * 0.5f;
    const SkRect rect =
        SkRect::MakeLTRB(box_left, top, box_left + box_w, top + box_h);

    SkPaint box;
    box.setAntiAlias(true);
    box.setColor(color);
    box.setAlphaf(box.getAlphaf() * lay.y_axis_opacity);
    canvas->drawRRect(SkRRect::MakeRectXY(rect, kCorner, kCorner), box);

    // Price text horizontally centered on the same axis center, and vertically
    // centered: shift the baseline so the glyph bounds' midpoint lands on `y`
    // (= the box center and the dotted line).
    SkPaint text_paint;
    text_paint.setAntiAlias(true);
    text_paint.setColor(chart.theme.colors[VROOM_COLOR_BADGE_TEXT]);
    text_paint.setAlphaf(text_paint.getAlphaf() * lay.y_axis_opacity);
    const float text_x = axis_center_x - text_w * 0.5f;
    const float baseline_y = y - (tb.fTop + tb.fBottom) * 0.5f;
    canvas->drawString(buf, text_x, baseline_y, font, text_paint);
}

}  // namespace vroom::price_indicator
