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
#include "labels.h"
#include "price_format.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::crosshair {

namespace {
constexpr float kRingRadius = 3.5f;  // hollow dot at the intersection
constexpr SkScalar kDash[2] = {2.f, 2.f};
constexpr float kPadV = 4.f;     // badge padding above/below the text
constexpr float kPadH = 8.f;     // badge padding left/right of the text
constexpr float kCorner = 6.f;   // rounded-badge corner radius
static_assert(kPadH == vroom::labels::kAxisBadgePadH,
              "the stroke ends on the pad this badge actually draws");

// Draws a filled rounded-rect badge with `text` centered on (`cx`, `cy`).
// Matches the current-price indicator's geometry (glyph-bounds centering so the
// digits sit on the center regardless of the font's cap-height metric).
void draw_badge(SkCanvas* canvas,
                const SkFont& font,
                const char* text,
                float cx,
                float cy,
                SkColor fill,
                SkColor text_color,
                float opacity) {
    if (opacity <= 0.f) return;
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
    box.setAlphaf(box.getAlphaf() * opacity);
    canvas->drawRRect(SkRRect::MakeRectXY(rect, kCorner, kCorner), box);

    SkPaint text_paint;
    text_paint.setAntiAlias(true);
    text_paint.setColor(text_color);
    text_paint.setAlphaf(text_paint.getAlphaf() * opacity);
    canvas->drawString(text, cx - text_w * 0.5f,
                       cy - (tb.fTop + tb.fBottom) * 0.5f, font, text_paint);
}

bool axis_font(const VroomChart& chart, SkFont* out) {
    auto tf = vroom::axis_typeface();
    if (!tf) return false;
    *out = SkFont(tf, vroom::badge_font_px(chart.theme));
    out->setSubpixel(true);
    out->setEdging(SkFont::Edging::kSubpixelAntiAlias);
    return true;
}

// Without the axis font the plus-only pill still needs a size.
constexpr float kFallbackGlyph = 10.f;

// The crosshair's price badge at the horizontal line — the plain badge, or the
// merged plus-and-price pill when the plus button is on. One measurement feeds
// both the draw and the hit test, so the two can't drift apart.
struct PriceBadge {
    bool shown = false;      // false: no badge and no button
    char text[48] = {};      // empty when only the plus draws (hidden y-axis)
    float text_x = 0.f;      // baseline origin
    float text_y = 0.f;
    float glyph = 0.f;       // plus side; 0 when the button is off
    float opacity = 1.f;
    crosshair_button::Rect rect{};
};

PriceBadge measure_price_badge(const VroomChart& chart,
                               const Layout& lay,
                               const PriceBounds& bounds,
                               float cy,
                               float candle_right,
                               const SkFont* font) {
    PriceBadge b;
    const bool button = chart.crosshair_button_style.enabled != 0;
    const bool axis =
        font && lay.y_axis_width_px > 0.f && lay.y_axis_opacity > 0.f;
    if (!axis && !button) return b;

    // The plus matches the digits' height, so it reads at the price's size.
    float glyph = kFallbackGlyph;
    if (font) {
        SkRect digit;
        font->measureText("0", 1, SkTextEncoding::kUTF8, &digit);
        glyph = digit.height();
    }

    float text_left = candle_right - kPadH;
    float text_right = text_left;
    float box_h = glyph + 2.f * kPadV;
    if (axis) {
        const double price = vroom::y_to_price(lay, bounds, cy);
        const vroom::PriceFormat fmt = vroom::with_tick_guard(
            chart.price_fmt,
            vroom::price_label_interval(bounds, vroom::price_pane_bottom(lay)));
        vroom::format_price(b.text, sizeof(b.text), price, fmt);
        SkRect tb;
        const float text_w = font->measureText(b.text, std::strlen(b.text),
                                               SkTextEncoding::kUTF8, &tb);
        text_left = vroom::labels::axis_badge_left(
                        lay.width_px, lay.y_axis_width_px, text_w) +
                    kPadH;
        text_right = text_left + text_w;
        box_h = tb.height() + 2.f * kPadV;
        b.text_x = text_left;
        b.text_y = cy - (tb.fTop + tb.fBottom) * 0.5f;
        b.opacity = lay.y_axis_opacity;
    }

    b.shown = true;
    b.glyph = button ? glyph : 0.f;
    const float gap = button && axis ? crosshair_button::kPlusGap : 0.f;
    b.rect = crosshair_button::pill_rect(text_left, text_right, cy, box_h,
                                         b.glyph, kPadH, gap);
    return b;
}

SkColor brighten(SkColor c, float k) {
    auto ch = [k](U8CPU v) {
        return static_cast<U8CPU>(std::clamp(v * k, 0.f, 255.f));
    };
    return SkColorSetARGB(SkColorGetA(c), ch(SkColorGetR(c)),
                          ch(SkColorGetG(c)), ch(SkColorGetB(c)));
}

void draw_price_badge(SkCanvas* canvas,
                      const VroomChart& chart,
                      const SkFont* font,
                      const PriceBadge& b) {
    if (!b.shown || b.opacity <= 0.f) return;
    const auto& colors = chart.theme.colors;
    const VroomCrosshairButtonStyle& s = chart.crosshair_button_style;
    const SkColor text_color = colors[VROOM_COLOR_BADGE_TEXT];
    SkColor fill = colors[VROOM_COLOR_CROSSHAIR_TARGET];
    const SkRect rect =
        SkRect::MakeLTRB(b.rect.left, b.rect.top, b.rect.right, b.rect.bottom);
    float radius = kCorner;
    if (b.glyph > 0.f) {
        if (chart.crosshair_button_hovered || chart.crosshair_pinned) {
            fill = brighten(fill, s.hover_boost);
        }
        radius = crosshair_button::corner_radius(s, rect.height());
    }

    SkPaint box;
    box.setAntiAlias(true);
    box.setColor(fill);
    box.setAlphaf(box.getAlphaf() * b.opacity);
    canvas->drawRRect(SkRRect::MakeRectXY(rect, radius, radius), box);

    if (b.glyph > 0.f) {
        const float px = crosshair_button::plus_center_x(b.rect, b.glyph, kPadH);
        const float py = rect.centerY();
        const float arm = b.glyph * 0.5f;
        SkPaint plus;
        plus.setAntiAlias(true);
        plus.setStyle(SkPaint::kStroke_Style);
        plus.setStrokeWidth(std::clamp(b.glyph * 0.18f, 1.5f, 3.f));
        plus.setStrokeCap(SkPaint::kRound_Cap);
        plus.setColor(text_color);
        plus.setAlphaf(plus.getAlphaf() * b.opacity);
        canvas->drawLine(px - arm, py, px + arm, py, plus);
        canvas->drawLine(px, py - arm, px, py + arm, plus);
    }

    if (font && b.text[0] != '\0') {
        SkPaint text_paint;
        text_paint.setAntiAlias(true);
        text_paint.setColor(text_color);
        text_paint.setAlphaf(text_paint.getAlphaf() * b.opacity);
        canvas->drawString(b.text, b.text_x, b.text_y, *font, text_paint);
    }
}
}  // namespace

float line_y(const VroomChart& chart,
             const Layout& lay,
             const PriceBounds& bounds,
             float candle_area_h) {
    const float y = chart.crosshair_pinned
                        ? vroom::price_to_y(lay, bounds, chart.crosshair_pin_price)
                        : chart.crosshair_y_px;
    return std::clamp(y, 0.f, std::max(0.f, candle_area_h));
}

bool button_rect(const VroomChart& chart,
                 const Layout& lay,
                 const PriceBounds& bounds,
                 float candle_right,
                 float candle_area_h,
                 crosshair_button::Rect* out,
                 double* price) {
    if (!chart.crosshair_active || !chart.crosshair_button_style.enabled ||
        candle_right <= 0.f || candle_area_h <= 0.f) {
        return false;
    }
    const float cy = line_y(chart, lay, bounds, candle_area_h);
    SkFont font;
    const bool has_font = axis_font(chart, &font);
    const PriceBadge b = measure_price_badge(chart, lay, bounds, cy, candle_right,
                                             has_font ? &font : nullptr);
    if (out) *out = b.rect;
    if (price) *price = vroom::y_to_price(lay, bounds, cy);
    return true;
}

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
    const float cy = line_y(chart, lay, bounds, candle_area_h);

    const SkColor color = chart.theme.colors[VROOM_COLOR_CROSSHAIR];

    // The horizontal dash meets the price badge (merged with the plus button
    // when it's on). Measure that badge first so the stroke and the pill share
    // one left edge; the vertical line still stops at the plot edge.
    SkFont font;
    const bool tf = axis_font(chart, &font);
    const PriceBadge price_badge = measure_price_badge(
        chart, lay, bounds, cy, candle_right, tf ? &font : nullptr);
    const float h_right = price_badge.shown ? price_badge.rect.left : candle_right;

    // Dashed perpendicular lines. The vertical line runs the full height of the
    // candle + indicator region (down to vline_bottom) so it stays visible over
    // any below-chart panes.
    SkPaint dash;
    dash.setAntiAlias(true);
    dash.setColor(color);
    dash.setStrokeWidth(1.f);
    dash.setPathEffect(SkDashPathEffect::Make(kDash, 0.f));
    canvas->drawLine(cx, 0.f, cx, vline_bottom, dash);
    canvas->drawLine(0.f, cy, h_right, cy, dash);

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

    // Axis badges sit on top of the axis labels and the current-price indicator
    // since the crosshair is the last draw step. Text needs the axis typeface;
    // without it only a plus-only pill (when the button is on) draws.
    if (!tf) {
        draw_price_badge(canvas, chart, nullptr, price_badge);
        return;
    }

    const SkColor badge_fill = chart.theme.colors[VROOM_COLOR_CROSSHAIR_TARGET];
    const SkColor badge_text = chart.theme.colors[VROOM_COLOR_BADGE_TEXT];

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
                   badge_text, lay.x_axis_opacity);
    }

    // Price badge over the y-axis strip, centered on the horizontal line with
    // its text in the y-axis labels' column. It was measured against the same
    // string the stroke was.
    draw_price_badge(canvas, chart, &font, price_badge);
}

}  // namespace vroom::crosshair
