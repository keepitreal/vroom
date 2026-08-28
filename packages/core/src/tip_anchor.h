// Which slot the line chart's tip marker anchors to (see draw_close_tip in
// ma_overlay.cpp).
//
// The tip marks the newest close in the *series*, not the newest close on
// screen — the same rule the price indicator follows (price_indicator.cpp).
// Panning into history therefore carries the dot off the right edge with the
// candle it belongs to, where draw_close_tip's on-pane guard drops it.
//
// Every other line-chart layer draws the visible slice, so the tip is the one
// caller that has to reach past range.end. The slice is a view into the same
// contiguous candle buffer, so widening the slot count is enough to reach the
// newest candle — no second pointer.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_tip_anchor.cpp.

#pragma once

#include <cstddef>

namespace vroom::tip_anchor {

// How draw_close_tip should be called for one frame.
struct Anchor {
    // Slot count to pass in place of the visible count, so slot 0 resolves to
    // the newest candle in the series. Never 0 for a non-empty visible slice.
    std::size_t slot_count;
    // Whether the interval-morph capture still lines up with slot 0.
    bool use_morph;
};

// `range_start`/`range_end` are the visible slice's half-open bounds into a
// series of `total` candles; `morphing` is whether an interval morph is running.
//
// The capture taken at a timeframe switch is indexed from the newest *visible*
// candle, so it only pairs with slot 0 while the newest candle is the one at the
// right edge. Scrolled back, pairing them would interpolate the tip between two
// unrelated candles and strand the dot mid-pane for the length of the switch.
inline Anchor at(std::size_t range_start,
                 std::size_t range_end,
                 std::size_t total,
                 bool morphing) {
    // A caller with an empty or out-of-bounds slice has nothing to anchor to;
    // reporting 0 slots lets draw_close_tip take its existing early-out.
    if (range_start >= total) return Anchor{0, false};
    return Anchor{total - range_start, morphing && range_end >= total};
}

}  // namespace vroom::tip_anchor
