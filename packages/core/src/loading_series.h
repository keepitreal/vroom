// The synthetic series behind the loading skeleton.
//
// A random walk rather than a fixed pattern, so the skeleton carries the same
// silhouette statistics as a real series — bodies, wicks and volume all in
// plausible proportion. That is what keeps the hand-off into real data short:
// the two shapes are already in the same family, so the morph has little
// distance to cover.
//
// Generated once from a fixed seed and then *windowed* to whatever bar count a
// frame needs, never regenerated per mount or per resize. A walk that
// re-randomised would shimmer every time the chart was laid out, and a resize
// would rewrite history rather than reframe it.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_loading_series.cpp.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "vroom/vroom_chart.h"

namespace vroom::loading_series {

// Long enough to cover any plausible bar count at the narrowest candle width;
// callers take the tail they need.
constexpr std::size_t kLength = 256;

constexpr double kStartingPrice = 100.0;
constexpr int64_t kStepMs = 3600000;  // one bar per hour

// Per-bar move, as a fraction of the open. Wide enough that bodies read at a
// glance, narrow enough that the walk doesn't run off to zero or the moon
// across kLength bars.
constexpr double kMinMove = 0.02;
constexpr double kMaxMove = 0.06;
constexpr double kWickRange = 0.03;

namespace detail {

// A small LCG (numerical recipes constants) rather than <random>, whose
// distributions are not specified to produce identical sequences across
// standard libraries. The skeleton has to look the same on iOS and in a
// browser, so the generator has to be ours.
struct Rng {
    uint32_t state;
    explicit Rng(uint32_t seed) : state(seed) {}
    // 0 .. 1, using the high bits — the low bits of an LCG have short periods.
    double next() {
        state = state * 1664525u + 1013904223u;
        return static_cast<double>(state >> 8) / 16777216.0;
    }
};

// Rounds to 2dp, matching the price precision a real feed would carry. Without
// it the axis labels under the skeleton would show more digits than the real
// series ever does.
inline double round2(double v) {
    return static_cast<double>(static_cast<int64_t>(v * 100.0 + 0.5)) / 100.0;
}

}  // namespace detail

// The full walk, ending at `end_ms`. Deterministic: same seed, same series.
inline std::vector<::VroomCandle> generate(int64_t end_ms, uint32_t seed = 0x5EEDu) {
    detail::Rng rng(seed);
    std::vector<::VroomCandle> out;
    out.reserve(kLength);

    double open = kStartingPrice;
    for (std::size_t i = 0; i < kLength; ++i) {
        // Magnitude and sign drawn separately so the walk has no drift: an
        // unsigned move scaled by +/-1 is symmetric, where a signed uniform
        // would bias toward whichever end the range is wider on.
        const double magnitude = kMinMove + rng.next() * (kMaxMove - kMinMove);
        const double move = rng.next() < 0.5 ? -magnitude : magnitude;
        const double close = open * (1.0 + move);

        const double body_top = open > close ? open : close;
        const double body_bottom = open < close ? open : close;

        ::VroomCandle c{};
        c.time_ms = end_ms - static_cast<int64_t>(kLength - 1 - i) * kStepMs;
        c.open = detail::round2(open);
        c.close = detail::round2(close);
        c.high = detail::round2(body_top * (1.0 + rng.next() * kWickRange));
        c.low = detail::round2(body_bottom * (1.0 - rng.next() * kWickRange));
        // Tied to the size of the move so big bars carry big volume, the
        // correlation a real series has and the eye expects.
        c.volume = 100000.0 * (1.0 + magnitude * 2.0) * (0.3 + rng.next() * 0.4);
        out.push_back(c);

        open = close;  // next bar opens where this one closed
    }
    return out;
}

// The last `count` bars of `all` — the window a frame actually draws. Taking
// the tail (rather than the head) keeps the newest bar pinned at the right
// edge as the count changes, so a resize reframes the same walk instead of
// sliding it.
inline const ::VroomCandle* tail(const std::vector<::VroomCandle>& all,
                                 std::size_t count) {
    if (all.empty()) return nullptr;
    if (count >= all.size()) return all.data();
    return all.data() + (all.size() - count);
}

}  // namespace vroom::loading_series
