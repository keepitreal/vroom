// Where the current-price indicator sits mid-morph (see price_indicator.cpp).
//
// The indicator marks the latest close, so every tick moves it. Drawing it
// straight from that close makes it jump while the candle it belongs to eases
// into place, which reads as the badge coming loose from the chart. Instead it
// rides the same capture the candles do: morph_from[0] is the newest candle's
// outgoing geometry and interval_morph_t is the host's eased clock, so the
// indicator lands on the candle's close edge on every frame rather than only at
// the two ends.
//
// The y is deliberately the same expression close_vertex uses for slot 0
// (ma_overlay.cpp). Matching it by construction, rather than by giving the two
// the same duration, is what keeps the badge glued to the line chart's tip.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_price_indicator_anim.cpp.

#pragma once

#include <algorithm>

#include "viewport.h"

namespace vroom::price_indicator_anim {

// One frame of the indicator.
struct Level {
    float y;       // pixels, on the candle's close edge
    double price;  // what the badge should read
    float bull_t;  // 0 = the captured direction's color, 1 = the new one
};

namespace detail {
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// The price a captured band fraction stood for. The capture stores fractions so
// it survives a resize or a rescale, so recovering the price it came from needs
// the band it was measured against — which is why morph_from_bounds is kept.
inline double price_at_fraction(const PriceBounds& b, double frac) {
    return b.min + frac * (b.max - b.min);
}
}  // namespace detail

// `from` is the newest candle's capture, or null when nothing is morphing or
// the capture doesn't pair with the newest candle (panned into history — see
// tip_anchor.h, which the caller shares the pairing rule with). A null capture
// gives the settled values, which is what the indicator drew before it
// animated at all.
//
// `from_bounds` is the price band `from` was captured against
// (VroomChart::morph_from_bounds).
inline Level level_at(const Layout& lay,
                      const PriceBounds& bounds,
                      double close_new,
                      bool bull_new,
                      const CandleSnapshot* from,
                      const PriceBounds& from_bounds,
                      float morph_t) {
    const float to_y = vroom::price_to_y(lay, bounds, close_new);
    if (!from) return Level{to_y, close_new, 1.f};

    const float t = std::clamp(morph_t, 0.f, 1.f);

    // Two different spaces on purpose. The y interpolates in band fractions,
    // because that is the space the capture is in and the space the candles
    // move through — anything else would start the indicator off the pixel the
    // close occupied last frame. The price interpolates in price space, so both
    // ends read exactly the close they belong to even when this tick set a new
    // extreme and rescaled the band underneath. The two agree whenever the band
    // holds still, which is the overwhelmingly common case: y_at_fraction is
    // affine, so a fraction lerp and a price lerp are then the same function.
    const float y =
        detail::lerp(vroom::y_at_fraction(lay, from->close), to_y, t);
    const double close_old =
        detail::price_at_fraction(from_bounds, from->close);

    return Level{y, close_old + (close_new - close_old) * static_cast<double>(t),
                 from->bull == bull_new ? 1.f : t};
}

}  // namespace vroom::price_indicator_anim
