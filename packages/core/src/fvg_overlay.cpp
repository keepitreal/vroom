#include "fvg_overlay.h"

#include <algorithm>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include "chart.h"
#include "fonts.h"

namespace vroom::fvg_overlay {

namespace {

constexpr SkScalar kDotted[2] = {2.f, 2.f};
constexpr SkScalar kDashed[2] = {6.f, 4.f};
constexpr float kLabelPadX = 4.f;  // clearance between a label and a box edge

// Dash pattern for a VroomFairValueGaps::border_style. Null = solid.
sk_sp<SkPathEffect> dash_for(int32_t border_style) {
    if (border_style == 1) return SkDashPathEffect::Make(kDotted, 0.f);
    if (border_style == 2) return SkDashPathEffect::Make(kDashed, 0.f);
    return nullptr;
}

// One gap resolved to pixels. `bullish` picks which of the paired colors apply.
struct Box {
    SkRect rect;
    bool bullish;
};

// The time a box's right edge stops at.
//
// An unfilled box runs its configured length, or — when extended — to the end of
// the newest bar. A filled one stops at the far side of the bar that filled it,
// whichever of the two comes first, so the shading covers exactly the span the
// gap was open for.
int64_t right_edge_ms(const VroomChart& chart, const vroom::fvg::Gap& g) {
    const int64_t dur = chart.candle_duration_ms;
    int64_t right = chart.fvg.extend_boxes
                        ? chart.candles.back().time_ms + dur
                        : g.time_ms + static_cast<int64_t>(chart.fvg.box_length) * dur;
    if (g.filled_ms != 0) right = std::min(right, g.filled_ms + dur);
    return right;
}

// Resolves `g` to pixels, or returns false when it shouldn't draw at all —
// filled and set to disappear, degenerate, or entirely off to one side.
bool box_for(const VroomChart& chart,
             const vroom::Layout& lay,
             const vroom::PriceBounds& bounds,
             int64_t window_ms,
             float candle_right,
             const vroom::fvg::Gap& g,
             Box* out) {
    if (g.filled_ms != 0 && chart.fvg.delete_after_fill) return false;

    const int64_t right_ms = right_edge_ms(chart, g);
    if (right_ms <= g.time_ms) return false;

    const float left =
        vroom::x_at_time(lay, chart.visible_start_ms, window_ms, g.time_ms);
    const float right =
        vroom::x_at_time(lay, chart.visible_start_ms, window_ms, right_ms);
    if (right < 0.f || left > candle_right) return false;

    out->rect = SkRect::MakeLTRB(left, vroom::price_to_y(lay, bounds, g.top),
                                 right, vroom::price_to_y(lay, bounds, g.bottom));
    out->bullish = g.bullish;
    return true;
}

// Whether there is anything at all to draw, and the cache is safe to walk.
bool active(const VroomChart& chart, SkCanvas* canvas, float candle_right,
            float candle_area_h) {
    return canvas && candle_right > 0.f && candle_area_h > 0.f &&
           chart.fvg.enabled && !chart.candles.empty() &&
           !chart.fvg_cache.empty();
}

SkColor with_opacity(SkColor c, float opacity) {
    return SkColorSetA(c, static_cast<U8CPU>(SkColorGetA(c) * opacity));
}

}  // namespace

void draw_boxes(SkCanvas* canvas,
                const VroomChart& chart,
                const Layout& lay,
                const PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h) {
    if (!active(chart, canvas, candle_right, candle_area_h)) return;
    const VroomFairValueGaps& cfg = chart.fvg;

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(cfg.border_width > 0.f ? cfg.border_width : 1.f);
    border.setPathEffect(dash_for(cfg.border_style));

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    for (const vroom::fvg::Gap& g : chart.fvg_cache) {
        Box b;
        if (!box_for(chart, lay, bounds, window_ms, candle_right, g, &b)) continue;

        const SkColor base = b.bullish ? cfg.bullish_color : cfg.bearish_color;
        fill.setColor(with_opacity(base, cfg.opacity));
        canvas->drawRect(b.rect, fill);

        if (cfg.border_enabled) {
            border.setColor(b.bullish ? cfg.bullish_border_color
                                      : cfg.bearish_border_color);
            canvas->drawRect(b.rect, border);
        }
    }
    canvas->restore();
}

void draw_labels(SkCanvas* canvas,
                 const VroomChart& chart,
                 const Layout& lay,
                 const PriceBounds& bounds,
                 int64_t window_ms,
                 float candle_right,
                 float candle_area_h) {
    if (!active(chart, canvas, candle_right, candle_area_h)) return;
    const VroomFairValueGaps& cfg = chart.fvg;
    if (!cfg.labels_enabled || chart.fvg_label.empty()) return;

    auto tf = vroom::axis_typeface();
    if (!tf) return;
    const float size = cfg.label_font_size > 0.f
                           ? cfg.label_font_size
                           : chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX];
    SkFont font(tf, size);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    const std::string& text = chart.fvg_label;
    SkRect tb;
    const float text_w =
        font.measureText(text.data(), text.size(), SkTextEncoding::kUTF8, &tb);

    // Extended boxes all stop at the newest bar, so their labels go out into the
    // empty slots past it rather than on top of the shading.
    const float extend_x =
        cfg.extend_boxes
            ? vroom::x_at_time(lay, chart.visible_start_ms, window_ms,
                               chart.candles.back().time_ms +
                                   static_cast<int64_t>(cfg.label_distance + 1) *
                                       chart.candle_duration_ms)
            : 0.f;

    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    for (const vroom::fvg::Gap& g : chart.fvg_cache) {
        Box b;
        if (!box_for(chart, lay, bounds, window_ms, candle_right, g, &b)) continue;

        // A filled box stops early even under extend, so it keeps its label
        // inside; only boxes still running to the newest bar move theirs out —
        // and only while the pane has the room, since the newest candle sits
        // flush right until the user pans.
        const bool outside = cfg.extend_boxes && g.filled_ms == 0 &&
                             extend_x + text_w <= candle_right;
        // A box whose right end runs past the pane would have its label sliced
        // mid-glyph by the clip, so hold the text inside the plot instead.
        const float x = std::min(outside ? extend_x
                                         : b.rect.right() - kLabelPadX - text_w,
                                 candle_right - kLabelPadX - text_w);
        if (x + text_w < 0.f) continue;

        const SkColor border = b.bullish ? cfg.bullish_border_color
                                         : cfg.bearish_border_color;
        paint.setColor(SkColorGetA(cfg.label_color) != 0 ? cfg.label_color : border);
        canvas->drawString(text.c_str(), x,
                           b.rect.centerY() - (tb.fTop + tb.fBottom) * 0.5f, font,
                           paint);
    }
    canvas->restore();
}

}  // namespace vroom::fvg_overlay
