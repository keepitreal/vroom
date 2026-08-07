#include "price_lines.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "chart.h"
#include "fonts.h"
#include "price_line_layout.h"
#include "theme.h"
#include "viewport.h"

namespace vroom::price_lines {

namespace {

constexpr SkScalar kDotted[2] = {2.f, 2.f};
constexpr SkScalar kDashed[2] = {6.f, 4.f};
constexpr float kBorderWidth = 1.f;   // pill outline
constexpr float kGhostAlpha = 0.35f;  // pre-drag position marker
constexpr float kCloseInset = 6.f;    // X glyph inset within its square cell
constexpr float kCloseStroke = 1.5f;
constexpr float kDividerAlpha = 0.5f;

// The reference glyph whose tight bounds set every pill's height, so a group's
// segments stay the same height whether or not their text has descenders — and
// match the price badge, which measures digits.
constexpr char kHeightRef[] = "0";

// Dash pattern for a VroomPriceLine::line_style. Null = solid.
sk_sp<SkPathEffect> dash_for(int32_t line_style) {
    if (line_style == 1) return SkDashPathEffect::Make(kDotted, 0.f);
    if (line_style == 2) return SkDashPathEffect::Make(kDashed, 0.f);
    return nullptr;
}

// Brightens `c`'s channels by `mul` (clamped at white), leaving alpha alone.
// Used for the hovered segment so a line lifts under the cursor without
// changing size or position.
SkColor boost(SkColor c, float mul) {
    if (mul <= 1.f) return c;
    const auto up = [mul](U8CPU v) {
        return static_cast<U8CPU>(
            std::min(255.f, static_cast<float>(v) * mul + 0.5f));
    };
    return SkColorSetARGB(SkColorGetA(c), up(SkColorGetR(c)), up(SkColorGetG(c)),
                          up(SkColorGetB(c)));
}

// The price a line renders at: the live drag position while it's being dragged,
// otherwise its committed price. The committed value is never mutated — the host
// applies or rejects the drag by restating its lines.
double render_price(const VroomChart& chart, size_t i) {
    if (chart.dragged_price_line == static_cast<int32_t>(i)) {
        return chart.dragged_price_line_price;
    }
    return chart.price_lines[i].price;
}

// The label font: the axis typeface, emboldened (these labels are chrome that
// must stay legible over candles). Returns false when no typeface is loaded yet,
// in which case callers fall back to lines without labels.
bool label_font(const VroomChart& chart, SkFont* out) {
    auto tf = vroom::axis_typeface();
    if (!tf) return false;
    const float size = chart.price_line_style.font_size_px > 0.f
                           ? chart.price_line_style.font_size_px
                           : chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX];
    *out = SkFont(tf, size);
    out->setSubpixel(true);
    out->setEmbolden(true);
    out->setEdging(SkFont::Edging::kSubpixelAntiAlias);
    return true;
}

float text_width(const SkFont& font, const std::string& s) {
    if (s.empty()) return 0.f;
    return font.measureText(s.data(), s.size(), SkTextEncoding::kUTF8);
}

// Measures one line's label so both passes lay it out identically.
LabelMetrics metrics_for(const SkFont& font,
                         const VroomChart::StoredPriceLine& pl) {
    SkRect ref;
    font.measureText(kHeightRef, std::strlen(kHeightRef), SkTextEncoding::kUTF8,
                     &ref);

    LabelMetrics m;
    m.text_w = text_width(font, pl.text);
    m.quantity_w = text_width(font, pl.quantity);
    m.label_h = ref.height() + 2.f * kPadV;
    m.closable = (pl.flags & VROOM_PRICE_LINE_CLOSABLE) != 0;
    return m;
}

// Rounds only the corners on the group's outer edges, so butted segments read as
// one continuous pill with dividers rather than a row of separate lozenges.
SkRRect segment_rrect(const Rect& r, bool round_left, bool round_right) {
    const SkRect rect = SkRect::MakeLTRB(r.left, r.top, r.right, r.bottom);
    const float l = round_left ? kCorner : 0.f;
    const float rr = round_right ? kCorner : 0.f;
    // setRectRadii takes corners in UL, UR, LR, LL order.
    const SkVector radii[4] = {{l, l}, {rr, rr}, {rr, rr}, {l, l}};
    SkRRect out;
    out.setRectRadii(rect, radii);
    return out;
}

// Draws `s` horizontally centered in `cell` and vertically centered on `cy`,
// shifting the baseline so the glyphs' own midpoint lands on the line.
void draw_centered_text(SkCanvas* canvas,
                        const SkFont& font,
                        const std::string& s,
                        const Rect& cell,
                        float cy,
                        SkColor color) {
    if (s.empty()) return;
    SkRect tb;
    const float w =
        font.measureText(s.data(), s.size(), SkTextEncoding::kUTF8, &tb);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    const float cx = (cell.left + cell.right) * 0.5f;
    canvas->drawString(s.c_str(), cx - w * 0.5f,
                       cy - (tb.fTop + tb.fBottom) * 0.5f, font, paint);
}

// Fills a segment and strokes its 1px outline.
void draw_pill(SkCanvas* canvas,
               const Rect& r,
               bool round_left,
               bool round_right,
               SkColor fill,
               SkColor border) {
    const SkRRect rr = segment_rrect(r, round_left, round_right);

    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(fill);
    canvas->drawRRect(rr, bg);

    if (SkColorGetA(border) == 0) return;
    SkPaint outline;
    outline.setAntiAlias(true);
    outline.setColor(border);
    outline.setStyle(SkPaint::kStroke_Style);
    outline.setStrokeWidth(kBorderWidth);
    // Inset by half the stroke so the outline sits inside the segment and butted
    // neighbours don't double up along their shared edge.
    canvas->drawRRect(segment_rrect(
                          Rect{r.left + kBorderWidth * 0.5f,
                               r.top + kBorderWidth * 0.5f,
                               r.right - kBorderWidth * 0.5f,
                               r.bottom - kBorderWidth * 0.5f},
                          round_left, round_right),
                      outline);
}

// The close affordance: an X stroked inside its square cell.
void draw_close_icon(SkCanvas* canvas, const Rect& cell, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(kCloseStroke);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    // Keep the glyph square even if the cell isn't, and never let a tiny cell
    // invert the inset.
    const float inset =
        std::min(kCloseInset, std::min(cell.width(), cell.bottom - cell.top) * 0.5f);
    const float l = cell.left + inset;
    const float r = cell.right - inset;
    const float t = cell.top + inset;
    const float b = cell.bottom - inset;
    canvas->drawLine(l, t, r, b, paint);
    canvas->drawLine(r, t, l, b, paint);
}

// A 1px vertical rule on a segment's left edge, separating butted segments.
void draw_divider(SkCanvas* canvas, const Rect& seg, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(false);
    paint.setColor(SkColorSetA(
        color, static_cast<U8CPU>(SkColorGetA(color) * kDividerAlpha)));
    paint.setStrokeWidth(1.f);
    canvas->drawLine(seg.left, seg.top + kBorderWidth, seg.left,
                     seg.bottom - kBorderWidth, paint);
}

// The price badge in the y-axis strip, matching the current-price indicator's
// geometry so the two read as the same chrome.
void draw_axis_badge(SkCanvas* canvas,
                     const SkFont& font,
                     const Layout& lay,
                     double price,
                     float y,
                     SkColor fill) {
    if (lay.y_axis_width_px <= 0.f) return;

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", price);
    const size_t len = std::strlen(buf);

    SkRect tb;
    const float text_w = font.measureText(buf, len, SkTextEncoding::kUTF8, &tb);
    const float box_w = text_w + 2.f * kPadH;
    const float box_h = tb.height() + 2.f * kPadV;
    const float cx = lay.width_px - lay.y_axis_width_px * 0.5f;
    const SkRect rect =
        SkRect::MakeXYWH(cx - box_w * 0.5f, y - box_h * 0.5f, box_w, box_h);

    SkPaint box;
    box.setAntiAlias(true);
    box.setColor(fill);
    canvas->drawRRect(SkRRect::MakeRectXY(rect, kCorner, kCorner), box);

    SkPaint text;
    text.setAntiAlias(true);
    text.setColor(SK_ColorWHITE);
    canvas->drawString(buf, cx - text_w * 0.5f,
                       y - (tb.fTop + tb.fBottom) * 0.5f, font, text);
}

}  // namespace

void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    if (chart.price_lines.empty()) return;

    const VroomPriceLineStyle& style = chart.price_line_style;
    SkFont font;
    const bool has_font = label_font(chart, &font);

    for (size_t i = 0; i < chart.price_lines.size(); ++i) {
        const VroomChart::StoredPriceLine& pl = chart.price_lines[i];
        const bool dragging = chart.dragged_price_line == static_cast<int32_t>(i);
        const bool hovered = chart.hovered_price_line == static_cast<int32_t>(i);

        const float y = vroom::price_to_y(lay, bounds, render_price(chart, i));
        if (y < 0.f || y > candle_area_h) continue;  // off-pane: cull, don't clamp

        const SkColor color = static_cast<SkColor>(pl.color);
        const float stroke = pl.width > 0.f ? pl.width : 1.f;
        // Hovering the line body lifts the whole line; hovering just the close
        // button lifts only that button (handled below).
        const bool line_hot = hovered && chart.hovered_price_line_part == 0;
        const SkColor line_color =
            line_hot || dragging ? boost(color, style.hover_boost) : color;

        SkPaint line;
        line.setAntiAlias(true);
        line.setColor(line_color);
        line.setStrokeWidth(stroke);
        line.setPathEffect(dash_for(pl.line_style));

        // A faint marker at the pre-drag price, so the user can see how far
        // they've moved and where releasing outside will snap back to.
        if (dragging) {
            const float ghost_y = vroom::price_to_y(lay, bounds, pl.price);
            if (ghost_y >= 0.f && ghost_y <= candle_area_h) {
                SkPaint ghost = line;
                ghost.setColor(SkColorSetA(
                    color, static_cast<U8CPU>(SkColorGetA(color) * kGhostAlpha)));
                canvas->drawLine(0.f, ghost_y, candle_right, ghost_y, ghost);
            }
        }

        const bool extend_left = (pl.flags & VROOM_PRICE_LINE_EXTEND_LEFT) != 0;

        LabelMetrics metrics;
        if (has_font) metrics = metrics_for(font, pl);
        const GroupLayout group = layout_group(metrics, y, candle_right, style);

        if (group.empty()) {
            // Nothing to anchor a partial span to, so a bare line always spans
            // the pane.
            canvas->drawLine(0.f, y, candle_right, y, line);
        } else {
            // Two segments with the label group punched out between them, so the
            // dashes don't show through the translucent pills.
            if (extend_left && group.left > 0.f) {
                canvas->drawLine(0.f, y, group.left, y, line);
            }
            if (group.right < candle_right) {
                canvas->drawLine(group.right, y, candle_right, y, line);
            }
        }

        if (!group.empty()) {
            const SkColor body_bg = static_cast<SkColor>(style.body_bg);
            const bool close_hot = hovered && chart.hovered_price_line_part == 1;

            // Only the group's outermost corners are rounded.
            const Rect* first = !group.body.empty()
                                    ? &group.body
                                    : (!group.quantity.empty() ? &group.quantity
                                                               : &group.close);
            const Rect* last = !group.close.empty()
                                   ? &group.close
                                   : (!group.quantity.empty() ? &group.quantity
                                                              : &group.body);

            if (!group.body.empty()) {
                draw_pill(canvas, group.body, first == &group.body,
                          last == &group.body, body_bg, line_color);
                draw_centered_text(canvas, font, pl.text, group.body, y,
                                   line_color);
            }
            if (!group.quantity.empty()) {
                // Solid fill + white text, so size reads at a glance against the
                // translucent body.
                draw_pill(canvas, group.quantity, first == &group.quantity,
                          last == &group.quantity, line_color, SK_ColorTRANSPARENT);
                draw_centered_text(canvas, font, pl.quantity, group.quantity, y,
                                   SK_ColorWHITE);
            }
            if (!group.close.empty()) {
                const SkColor close_color =
                    close_hot ? boost(color, style.hover_boost) : line_color;
                draw_pill(canvas, group.close, first == &group.close,
                          last == &group.close, body_bg, close_color);
                if (!group.quantity.empty()) {
                    draw_divider(canvas, group.close, close_color);
                }
                draw_close_icon(canvas, group.close, close_color);
            }
        }

        if (has_font && (pl.flags & VROOM_PRICE_LINE_AXIS_LABEL) != 0) {
            draw_axis_badge(canvas, font, lay, render_price(chart, i), y,
                            line_color);
        }
    }
}

HitResult hit_test(const VroomChart& chart,
                   const Layout& lay,
                   const PriceBounds& bounds,
                   float candle_right,
                   float candle_area_h,
                   float x,
                   float y) {
    HitResult miss{-1, -1};
    if (chart.price_lines.empty()) return miss;
    if (candle_right <= 0.f || candle_area_h <= 0.f) return miss;

    const VroomPriceLineStyle& style = chart.price_line_style;
    SkFont font;
    const bool has_font = label_font(chart, &font);

    // Close buttons first — they sit on top of the line, so a press there must
    // never be read as the start of a drag.
    int32_t best = -1;
    float best_dist = 0.f;
    for (size_t i = 0; i < chart.price_lines.size(); ++i) {
        const VroomChart::StoredPriceLine& pl = chart.price_lines[i];
        if ((pl.flags & VROOM_PRICE_LINE_CLOSABLE) == 0) continue;
        if (!has_font) continue;  // no font, no rendered button to press

        const float ly = vroom::price_to_y(lay, bounds, render_price(chart, i));
        if (ly < 0.f || ly > candle_area_h) continue;

        const GroupLayout group =
            layout_group(metrics_for(font, pl), ly, candle_right, style);
        if (!contains(group.close, x, y)) continue;

        const float dist = std::fabs(y - ly);
        if (best < 0 || dist < best_dist) {
            best = static_cast<int32_t>(i);
            best_dist = dist;
        }
    }
    if (best >= 0) return HitResult{best, 1};

    // Then draggable lines, nearest in y.
    for (size_t i = 0; i < chart.price_lines.size(); ++i) {
        const VroomChart::StoredPriceLine& pl = chart.price_lines[i];
        if ((pl.flags & VROOM_PRICE_LINE_DRAGGABLE) == 0) continue;

        const float ly = vroom::price_to_y(lay, bounds, render_price(chart, i));
        if (ly < 0.f || ly > candle_area_h) continue;
        if (!hits_line(ly, pl.width, y)) continue;

        // Don't grab where the line isn't drawn: without extend_left only the
        // label group and the run to the axis are live.
        if ((pl.flags & VROOM_PRICE_LINE_EXTEND_LEFT) == 0) {
            const GroupLayout group = layout_group(
                has_font ? metrics_for(font, pl) : LabelMetrics{}, ly,
                candle_right, style);
            if (!group.empty() && x < group.left) continue;
        }
        if (x < 0.f || x > candle_right) continue;

        const float dist = std::fabs(y - ly);
        if (best < 0 || dist < best_dist) {
            best = static_cast<int32_t>(i);
            best_dist = dist;
        }
    }
    if (best >= 0) return HitResult{best, 0};

    return miss;
}

}  // namespace vroom::price_lines
