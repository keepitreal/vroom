// Blending two packed ARGB colors.
//
// Shared because anything anchored to the newest candle has to cross-fade when
// that candle changes direction mid-morph: the body itself (candles.cpp) and
// the current-price indicator (price_indicator.cpp) both flip between the bull
// and bear accents, and a snap in either while the other eases is visible.
//
// Skia-free so the pure translation units can use it.

#pragma once

#include <cstdint>

namespace vroom {

// Channel-wise, including alpha. `t` is expected in [0,1].
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

}  // namespace vroom
