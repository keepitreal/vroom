#include "crosshair.h"

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

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "chart.h"
#include "fonts.h"
#include "price_format.h"
#include "theme.h"
#include "ticks.h"

namespace vroom::crosshair {

namespace {
constexpr float kRingRadius = 3.5f;  // hollow dot at the intersection
constexpr SkScalar kDash[2] = {2.f, 2.f};
constexpr float kPadV = 4.f;     // badge padding above/below the text
constexpr float kPadH = 8.f;     // badge padding left/right of the text
constexpr float kCorner = 6.f;   // rounded-badge corner radius

// Draws a filled rounded-rect badge with `text` centered on (`cx`, `cy`).
// Matches the current-price indicator's geometry (glyph-bounds centering so the
// digits sit on the center regardless of the font's cap-height metric).
void draw_badge(SkCanvas* canvas,
                const SkFont& font,
                const char* text,
                float cx,
                float cy,
                SkColor fill,
                SkColor text_color) {
    const size_t len = std::strlen(text);
    SkRect tb;
    const float text_w = font.measureText(text, len, SkTextEncoding::kUTF8, &tb);
    const float box_w = text_w + 2.f * kPadH;
    const float box_h = tb.height() + 2.f * kPadV;
    const SkRect rect = SkRect::MakeXYWH(cx - box_w * 0.5f, cy - box_h * 0.5f,
                                         box_w, box_h);

    SkPaint box;
    box.setAntiAlias(true);
    box.setColor(fill);
    canvas->drawRRect(SkRRect::MakeRectXY(rect, kCorner, kCorner), box);

    SkPaint text_paint;
    text_paint.setAntiAlias(true);
    text_paint.setColor(text_color);
    canvas->drawString(text, cx - text_w * 0.5f,
                       cy - (tb.fTop + tb.fBottom) * 0.5f, font, text_paint);
}
}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h,
          float vline_bottom,
          float snap_x,
          int64_t snap_time_ms) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;

    // Vertical line + ring snap to the nearest candle's center x; the horizontal
    // line and the ring's y follow the (lifted) touch y. Clamp into the candle
    // area so nothing bleeds into the axis strips.
    const float cx = std::clamp(snap_x, 0.f, candle_right);
    const float cy = std::clamp(chart.crosshair_y_px, 0.f, candle_area_h);

    const SkColor color = chart.theme.colors[VROOM_COLOR_CROSSHAIR];

    // Dashed perpendicular lines. The vertical line runs the full height of the
    // candle + indicator region (down to vline_bottom) so it stays visible over
    // any below-chart panes; the horizontal line spans from the left edge to the
    // y-axis strip at candle_right.
    SkPaint dash;
    dash.setAntiAlias(true);
    dash.setColor(color);
    dash.setStrokeWidth(1.f);
    dash.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(cx, 0.f, cx, vline_bottom, dash);
    canvas->drawLine(0.f, cy, candle_right, cy, dash);

    // Punch the dashes out from under the ring so its center reads as hollow.
    SkPaint hole;
    hole.setAntiAlias(true);
    hole.setColor(chart.theme.colors[VROOM_COLOR_BACKGROUND]);
    canvas->drawCircle(cx, cy, kRingRadius, hole);

    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setColor(chart.theme.colors[VROOM_COLOR_CROSSHAIR_TARGET]);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(2.f);  // thicker border so the dot reads clearly
    canvas->drawCircle(cx, cy, kRingRadius, ring);

    // Axis badges (neutral grey, white text) sit on top of the axis labels and
    // the current-price indicator since the crosshair is the last draw step.
    // They need the axis typeface; if it isn't loaded the lines alone suffice.
    auto tf = vroom::axis_typeface();
    if (!tf) return;

    SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    const SkColor badge_fill = chart.theme.colors[VROOM_COLOR_CROSSHAIR_TARGET];

    // Date/time badge over the x-axis strip, centered on the vertical line and
    // vertically aligned with the time labels (centered in the strip).
    if (lay.x_axis_height_px > 0.f) {
        char buf[32];
        const time_t time_s = static_cast<time_t>(snap_time_ms / 1000);
        struct tm tm_buf;
        localtime_r(&time_s, &tm_buf);
        std::snprintf(buf, sizeof(buf), "%02d/%02d %02d:%02d",
                      tm_buf.tm_mon + 1, tm_buf.tm_mday, tm_buf.tm_hour,
                      tm_buf.tm_min);

        const float strip_center_y = vline_bottom + lay.x_axis_height_px * 0.5f;
        // Keep the badge within the candle area so it never slides under the
        // y-axis price column.
        const size_t len = std::strlen(buf);
        const float text_w = font.measureText(buf, len, SkTextEncoding::kUTF8);
        const float half_w = (text_w + 2.f * kPadH) * 0.5f;
        const float badge_cx =
            std::clamp(cx, half_w, std::max(half_w, candle_right - half_w));
        draw_badge(canvas, font, buf, badge_cx, strip_center_y, badge_fill,
                   SK_ColorWHITE);
    }

    // Price badge over the y-axis strip, centered on the horizontal line and
    // sharing the y-axis labels' column.
    if (lay.y_axis_width_px > 0.f) {
        const double price = vroom::y_to_price(lay, bounds, cy);
        char buf[48];
        const vroom::PriceFormat fmt = vroom::with_tick_guard(
            chart.price_fmt,
            vroom::pick_price_interval(bounds.max - bounds.min,
                                       vroom::price_pane_bottom(lay)));
        vroom::format_price(buf, sizeof(buf), price, fmt);
        const float axis_center_x = lay.width_px - lay.y_axis_width_px * 0.5f;
        draw_badge(canvas, font, buf, axis_center_x, cy, badge_fill,
                   SK_ColorWHITE);
    }
}

}  // namespace vroom::crosshair
