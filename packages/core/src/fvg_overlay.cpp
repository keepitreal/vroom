#include "fvg_overlay.h"

#include <algorithm>
#include <array>
#include <cstddef>
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

// A stretch of time one box covers. A gap draws one of these while it is open
// and, under show_inverse, a second of the opposite polarity once it has been
// violated.
struct Span {
    int64_t start_ms;
    int64_t end_ms;
    bool bullish;  // polarity of THIS span, already flipped for an inverse
    bool inverse;
    bool open;     // still running at the newest bar rather than cut short
};

// The spans `g` draws, written to `out` oldest first, returning how many. Zero
// when the gap is set to disappear on the fill and has no inversion to show.
std::size_t spans_for(const VroomChart& chart,
                      const vroom::fvg::Gap& g,
                      std::array<Span, 2>& out) {
    const VroomFairValueGaps& cfg = chart.fvg;
    const int64_t dur = chart.candle_duration_ms;
    const int64_t newest_end = chart.candles.back().time_ms + dur;
    const int64_t length = static_cast<int64_t>(cfg.box_length) * dur;
    std::size_t n = 0;

    // The original. It runs its configured length, or — when extended — to the
    // end of the newest bar, and stops at the far side of the bar that filled
    // it, so the shading covers exactly the span the gap was open for.
    if (g.filled_ms == 0 || !cfg.delete_after_fill) {
        int64_t end = cfg.extend_boxes ? newest_end : g.time_ms + length;
        const bool open = g.filled_ms == 0;
        if (!open) end = std::min(end, g.filled_ms + dur);
        if (end > g.time_ms) out[n++] = Span{g.time_ms, end, g.bullish, false, open};
    }

    // The inversion, anchored past the close of the breaking bar so it picks up
    // exactly where the original stops and the two never overlap. It measures
    // its own length from there and ends where price reclaimed the band.
    if (cfg.show_inverse && g.filled_ms != 0) {
        const int64_t start = g.filled_ms + dur;
        int64_t end = cfg.extend_boxes ? newest_end : start + length;
        const bool open = g.invalidated_ms == 0;
        if (!open) end = std::min(end, g.invalidated_ms + dur);
        if (end > start) out[n++] = Span{start, end, !g.bullish, true, open};
    }
    return n;
}

// One span resolved to pixels, or false when it lands entirely off to one side.
bool box_for(const VroomChart& chart,
             const vroom::Layout& lay,
             const vroom::PriceBounds& bounds,
             int64_t window_ms,
             float candle_right,
             const vroom::fvg::Gap& g,
             const Span& s,
             SkRect* out) {
    const float left =
        vroom::x_at_time(lay, chart.visible_start_ms, window_ms, s.start_ms);
    const float right =
        vroom::x_at_time(lay, chart.visible_start_ms, window_ms, s.end_ms);
    if (right < 0.f || left > candle_right) return false;

    *out = SkRect::MakeLTRB(left, vroom::price_to_y(lay, bounds, g.top), right,
                            vroom::price_to_y(lay, bounds, g.bottom));
    return true;
}

// The fill color a span shades with, before opacity.
SkColor fill_color(const VroomFairValueGaps& cfg, const Span& s) {
    if (s.inverse) {
        return s.bullish ? cfg.inverse_bullish_color : cfg.inverse_bearish_color;
    }
    return s.bullish ? cfg.bullish_color : cfg.bearish_color;
}

// The color a span's border and label take. Inverse spans have no paired border
// color of their own, so they reuse their fill at full alpha — the same
// relationship bullishBorderColor already has with bullishColor by default.
SkColor line_color(const VroomFairValueGaps& cfg, const Span& s) {
    if (s.inverse) return SkColorSetA(fill_color(cfg, s), 0xff);
    return s.bullish ? cfg.bullish_border_color : cfg.bearish_border_color;
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
    std::array<Span, 2> spans;
    for (const vroom::fvg::Gap& g : chart.fvg_cache) {
        const std::size_t n = spans_for(chart, g, spans);
        for (std::size_t i = 0; i < n; ++i) {
            SkRect rect;
            if (!box_for(chart, lay, bounds, window_ms, candle_right, g, spans[i],
                         &rect)) {
                continue;
            }

            fill.setColor(with_opacity(fill_color(cfg, spans[i]), cfg.opacity));
            canvas->drawRect(rect, fill);

            if (cfg.border_enabled) {
                border.setColor(line_color(cfg, spans[i]));
                canvas->drawRect(rect, border);
            }
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
    const bool any_inverse = cfg.show_inverse && !chart.fvg_inverse_label.empty();
    if (!cfg.labels_enabled || (chart.fvg_label.empty() && !any_inverse)) return;

    auto tf = vroom::axis_typeface();
    if (!tf) return;
    const float size = cfg.label_font_size > 0.f
                           ? cfg.label_font_size
                           : chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX];
    SkFont font(tf, size);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    // Indexed by Span::inverse, so a span picks its own text and metrics.
    struct Label {
        const std::string* text;
        float width;
        SkRect bounds;
    };
    Label labels[2] = {{&chart.fvg_label, 0.f, {}},
                       {&chart.fvg_inverse_label, 0.f, {}}};
    for (Label& l : labels) {
        l.width = font.measureText(l.text->data(), l.text->size(),
                                   SkTextEncoding::kUTF8, &l.bounds);
    }

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
    std::array<Span, 2> spans;
    for (const vroom::fvg::Gap& g : chart.fvg_cache) {
        const std::size_t n = spans_for(chart, g, spans);
        for (std::size_t i = 0; i < n; ++i) {
            const Span& s = spans[i];
            const Label& l = labels[s.inverse ? 1 : 0];
            if (l.text->empty()) continue;

            SkRect rect;
            if (!box_for(chart, lay, bounds, window_ms, candle_right, g, s,
                         &rect)) {
                continue;
            }

            // A box that was cut short stops early even under extend, so it
            // keeps its label inside; only boxes still running to the newest
            // bar move theirs out — and only while the pane has the room, since
            // the newest candle sits flush right until the user pans.
            const bool outside = cfg.extend_boxes && s.open &&
                                 extend_x + l.width <= candle_right;
            // A box whose right end runs past the pane would have its label
            // sliced mid-glyph by the clip, so hold the text inside the plot.
            const float x =
                std::min(outside ? extend_x : rect.right() - kLabelPadX - l.width,
                         candle_right - kLabelPadX - l.width);
            if (x + l.width < 0.f) continue;

            paint.setColor(SkColorGetA(cfg.label_color) != 0 ? cfg.label_color
                                                             : line_color(cfg, s));
            canvas->drawString(
                l.text->c_str(), x,
                rect.centerY() - (l.bounds.fTop + l.bounds.fBottom) * 0.5f, font,
                paint);
        }
    }
    canvas->restore();
}

}  // namespace vroom::fvg_overlay
