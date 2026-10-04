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
#include "color_lerp.h"
#include "fonts.h"
#include "labels.h"
#include "price_format.h"
#include "price_indicator_anim.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::price_indicator {

namespace {
constexpr SkScalar kDash[2] = {2.f, 2.f};
constexpr float kPadV = 4.f;     // box padding above/below the text
constexpr float kPadH = 8.f;     // box padding left/right of the text
constexpr float kCorner = 6.f;   // rounded-box corner radius
static_assert(kPadH == vroom::labels::kAxisBadgePadH,
              "the stroke ends on the pad this badge actually draws");
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h,
          const vroom::CandleSnapshot* morph_from,
          float morph_t) {
    if (!canvas || chart.candles.empty()) return;

    // The "current price" is the latest period's close, regardless of whether
    // that candle is horizontally in view. Mid-morph it is wherever that close
    // has eased to, so the badge stays on the candle's close edge.
    const ::VroomCandle& last = chart.candles.back();
    const bool bull = last.close >= last.open;
    const auto level = vroom::price_indicator_anim::level_at(
        lay, bounds, last.close, bull, morph_from, chart.morph_from_bounds,
        morph_t);

    // Blended rather than switched, so a candle that changes direction under a
    // tick carries the indicator's color with it instead of snapping — the same
    // cross-fade the body itself does.
    const SkColor color = static_cast<SkColor>(vroom::lerp_argb(
        chart.theme.colors[bull ? VROOM_COLOR_ACCENT_BEAR : VROOM_COLOR_ACCENT_BULL],
        chart.theme.colors[bull ? VROOM_COLOR_ACCENT_BULL : VROOM_COLOR_ACCENT_BEAR],
        level.bull_t));

    const float y = level.y;
    if (y < 0.f || y > candle_area_h) return;  // price scrolled off-range

    // Price box in the y-axis strip. Needs the axis typeface; if it isn't
    // loaded yet, the line alone still conveys the level. Same when the y-axis
    // is hidden — the level stays readable, it just loses its badge.
    auto tf = vroom::axis_typeface();
    const bool show_badge = tf && lay.y_axis_opacity > 0.f;
    const bool join_badge = show_badge && lay.y_axis_width_px > 0.f;

    SkFont font;
    char buf[48];
    size_t len = 0;
    SkRect tb = SkRect::MakeEmpty();
    float text_w = 0.f;
    if (show_badge) {
        font = SkFont(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        const vroom::PriceFormat fmt = vroom::with_tick_guard(
            chart.price_fmt,
            vroom::price_label_interval(bounds, candle_area_h));
        vroom::format_price(buf, sizeof(buf), level.price, fmt);
        len = std::strlen(buf);
        // Tight glyph bounds (origin at the baseline) so the digits center on
        // `y` even when the font's cap-height metric misses the digit extent.
        text_w = font.measureText(buf, len, SkTextEncoding::kUTF8, &tb);
    }

    // Meet the badge when it will draw. Otherwise stop at the plot edge.
    const float line_right = join_badge
        ? vroom::labels::axis_badge_left(lay.width_px, lay.y_axis_width_px, text_w)
        : candle_right;
    SkPaint line;
    line.setAntiAlias(true);
    line.setColor(color);
    line.setStrokeWidth(1.f);
    line.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(0.f, y, line_right, y, line);
    if (!show_badge) return;

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
