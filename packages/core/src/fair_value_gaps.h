// Fair Value Gaps over a candle series — pure, no Skia, so it builds into the
// unit-test target. Drawn as shaded price-pane boxes over three-candle
// imbalances.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::fvg {

// One detected imbalance, anchored in absolute time and price so the renderer
// can place it without knowing which bar it came from.
struct Gap {
    int64_t time_ms = 0;    // open time of the middle bar
    double  top = 0.0;      // upper price edge
    double  bottom = 0.0;   // lower price edge
    bool    bullish = false;
    int64_t filled_ms = 0;  // open time of the bar that filled it; 0 = unfilled
};

// Which price settles that a gap has been traded back through.
enum FillType : int {
    kFillClose = 0,  // a candle must close past the far edge
    kFillWick = 1,   // a high or low reaching through is enough
};

// Detects the gaps in the last `max_bars_back` bars of [candles, candles+n),
// oldest first, and resolves each one's fill.
//
// A gap sits around the middle bar of a three-candle run whose outer wicks miss
// each other: bullish when candles[i-1].high < candles[i+1].low (spanning that
// range), bearish when candles[i-1].low > candles[i+1].high. `wait_for_close`
// withholds a gap whose third candle is still the newest, live bar.
//
// The fill scan walks forward from i+2 and records the first bar to reach the
// far edge — the bottom of a bullish gap, the top of a bearish one. It always
// runs, so hiding versus truncating a filled box stays a drawing concern.
void compute(const ::VroomCandle* candles, std::size_t n, int max_bars_back,
             bool wait_for_close, int fill_type, std::vector<Gap>& out);

}  // namespace vroom::fvg
