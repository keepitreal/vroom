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

// Channel-wise ARGB blend, for a candle that changes direction mid-morph.
inline uint32_t lerp_argb(uint32_t a, uint32_t b, float t) {
    uint32_t out = 0;
    for (int shift = 0; shift < 32; shift += 8) {
        const float ca = static_cast<float>((a >> shift) & 0xFFu);
        const float cb = static_cast<float>((b >> shift) & 0xFFu);
        const auto v = static_cast<uint32_t>(ca + (cb - ca) * t + 0.5f);
        out |= v << shift;
    }
    return out;
}

inline uint32_t scale_alpha(uint32_t argb, float a) {
    const auto alpha =
        static_cast<uint32_t>(static_cast<float>((argb >> 24) & 0xFFu) * a + 0.5f);
    return (alpha << 24) | (argb & 0x00FFFFFFu);
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// One slot's wick/body extents in pixels.
struct SlotGeom {
    float x, open, high, low, close;
};
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
          float opacity,
          const CandleSnapshot* from,
          std::size_t from_n,
          float morph_t) {
    if (!canvas) return;

    collapse = std::clamp(collapse, 0.f, 1.f);
    opacity = std::clamp(opacity, 0.f, 1.f);
    morph_t = std::clamp(morph_t, 0.f, 1.f);
    if (opacity <= 0.f) return;

    // A finished morph drops the capture, reducing the loop below to the plain
    // candle path.
    const std::size_t from_count = vroom::morph_from_count(from, from_n, morph_t);
    const std::size_t slots = std::max(n, from_count);
    if (slots == 0) return;

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
    const uint32_t wick_color_bull =
        resolve_color(theme.colors[VROOM_COLOR_WICK_BULL], fill_bull);
    const uint32_t wick_color_bear =
        resolve_color(theme.colors[VROOM_COLOR_WICK_BEAR], fill_bear);
    const uint32_t border_color_bull =
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BULL], fill_bull);
    const uint32_t border_color_bear =
        resolve_color(theme.colors[VROOM_COLOR_BORDER_BEAR], fill_bear);

    // Draw the border only when it differs from the fill; an inherited/transparent
    // border color (the default, and the "hide" path) renders nothing — so a hidden
    // border costs no pixels and never affects candle width.
    const bool draw_border_bull = border_color_bull != fill_bull;
    const bool draw_border_bear = border_color_bear != fill_bear;

    SkPaint bull_paint;
    bull_paint.setAntiAlias(true);
    bull_paint.setColor(fill_bull);

    SkPaint bear_paint;
    bear_paint.setAntiAlias(true);
    bear_paint.setColor(fill_bear);

    SkPaint wick_bull;
    wick_bull.setAntiAlias(true);
    wick_bull.setColor(wick_color_bull);
    wick_bull.setStrokeWidth(theme.floats[VROOM_FLOAT_WICK_WIDTH_PX]);
    wick_bull.setStyle(SkPaint::kStroke_Style);
    wick_bull.setStrokeCap(wick_round ? SkPaint::kRound_Cap : SkPaint::kButt_Cap);

    SkPaint wick_bear = wick_bull;
    wick_bear.setColor(wick_color_bear);

    // 1px body outlines, drawn *inside* the body (see the inset at draw time) so
    // a contrasting border never widens the candle. Default sentinel resolves to
    // the fill color; such borders are skipped entirely (draw_border_* above).
    SkPaint border_bull;
    border_bull.setAntiAlias(true);
    border_bull.setStyle(SkPaint::kStroke_Style);
    border_bull.setStrokeWidth(kBorderWidthPx);
    border_bull.setColor(border_color_bull);

    SkPaint border_bear = border_bull;
    border_bear.setColor(border_color_bear);

    // Scratch paints for the slots that need a per-slot color (see below).
    SkPaint slot_wick, slot_fill, slot_border;

    const float half_body = body_w * 0.5f;
    const float area_w = vroom::candle_area_width(lay);

    // Iterate slots counting back from the right edge — the pairing a timeframe
    // switch preserves — descending so the paint order stays oldest-first and
    // touching wicks/borders composite exactly as they do without a morph.
    for (std::size_t k = slots; k-- > 0;) {
        const ::VroomCandle* to = (k < n) ? &visible[n - 1 - k] : nullptr;
        const CandleSnapshot* frm = (k < from_count) ? &from[k] : nullptr;

        SlotGeom g{};
        bool bull;
        float alpha = 1.f;

        if (to) {
            const SlotGeom t{
                vroom::candle_center_x(lay, to->time_ms, candle_duration_ms,
                                       visible_start_ms, window_ms),
                vroom::price_to_y(lay, bounds, to->open),
                vroom::price_to_y(lay, bounds, to->high),
                vroom::price_to_y(lay, bounds, to->low),
                vroom::price_to_y(lay, bounds, to->close),
            };
            bull = to->close >= to->open;
            if (frm) {
                // The snapshot is in band fractions, so it lands on the same
                // pixels it occupied pre-switch even though the bounds changed.
                g.x = lerp(frm->x * area_w, t.x, morph_t);
                g.open = lerp(vroom::y_at_fraction(lay, frm->open), t.open, morph_t);
                g.high = lerp(vroom::y_at_fraction(lay, frm->high), t.high, morph_t);
                g.low = lerp(vroom::y_at_fraction(lay, frm->low), t.low, morph_t);
                g.close =
                    lerp(vroom::y_at_fraction(lay, frm->close), t.close, morph_t);
            } else {
                g = t;
                // A slot with no counterpart: fade in rather than pop.
                if (from_count > 0) alpha = morph_t;
            }
        } else {
            g = {frm->x * area_w,
                 vroom::y_at_fraction(lay, frm->open),
                 vroom::y_at_fraction(lay, frm->high),
                 vroom::y_at_fraction(lay, frm->low),
                 vroom::y_at_fraction(lay, frm->close)};
            bull = frm->bull;
            alpha = 1.f - morph_t;  // the new data dropped this slot
        }
        if (alpha <= 0.f) continue;

        // Collapse high/low/open toward the close so the candle folds into its
        // close point (the line vertex) as `collapse` → 1.
        const float y_close = g.close;
        const float y_high = g.high * (1.f - collapse) + y_close * collapse;
        const float y_low = g.low * (1.f - collapse) + y_close * collapse;
        const float y_open = g.open * (1.f - collapse) + y_close * collapse;

        // Fast path: the prebuilt bull/bear paints. A faded slot and a candle
        // that changes direction mid-morph both need per-slot colors.
        const SkPaint* wick_p = bull ? &wick_bull : &wick_bear;
        const SkPaint* fill_p = bull ? &bull_paint : &bear_paint;
        const SkPaint* border_p = bull ? &border_bull : &border_bear;
        bool draw_border = bull ? draw_border_bull : draw_border_bear;

        const bool flip = frm && to && frm->bull != bull;
        if (flip || alpha < 0.999f) {
            const auto blend = [&](uint32_t c_bull, uint32_t c_bear) {
                uint32_t c = bull ? c_bull : c_bear;
                if (flip) c = lerp_argb(bull ? c_bear : c_bull, c, morph_t);
                return scale_alpha(c, alpha);
            };
            slot_wick = *wick_p;
            slot_wick.setColor(blend(wick_color_bull, wick_color_bear));
            slot_fill = *fill_p;
            slot_fill.setColor(blend(fill_bull, fill_bear));
            slot_border = *border_p;
            slot_border.setColor(blend(border_color_bull, border_color_bear));
            wick_p = &slot_wick;
            fill_p = &slot_fill;
            border_p = &slot_border;
            if (flip) draw_border = draw_border_bull || draw_border_bear;
        }

        canvas->drawLine(g.x, y_high, g.x, y_low, *wick_p);

        const float y_top = std::min(y_open, y_close);
        const float y_bot = std::max(y_open, y_close);
        const float h = std::max(1.f, y_bot - y_top);
        const SkRect body = SkRect::MakeXYWH(g.x - half_body, y_top, body_w, h);
        // Corner radius clamped so thin candles / short bodies don't over-round.
        const float r = std::min({body_r, body_w * 0.5f, h * 0.5f});
        if (r > 0.f) {
            canvas->drawRoundRect(body, r, r, *fill_p);
        } else {
            canvas->drawRect(body, *fill_p);
        }

        // Inset by half the stroke so the centered stroke falls fully within the
        // body: the border overlaps the fill's outer edge instead of straddling
        // it, keeping the candle's footprint exactly body_w. Skip when the body is
        // too small to hold it.
        if (draw_border && body_w > kBorderWidthPx && h > kBorderWidthPx) {
            const SkRect inner =
                body.makeInset(kBorderWidthPx * 0.5f, kBorderWidthPx * 0.5f);
            // Concentric radius so the inside border follows the rounded body.
            const float ir = std::max(0.f, r - kBorderWidthPx * 0.5f);
            if (ir > 0.f) {
                canvas->drawRoundRect(inner, ir, ir, *border_p);
            } else {
                canvas->drawRect(inner, *border_p);
            }
        }
    }

    if (fade_layer) canvas->restore();
}

}  // namespace vroom::candles
