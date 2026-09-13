// Average True Range — pure computation, no Skia. Kept Skia-free so it builds
// into the unit-test target.
//
// True Range is the widest of the bar's own high-low span and the two gaps from
// its extremes to the previous close, so an overnight jump the bar's range
// misses still counts. ATR is that series smoothed over `period` bars.

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::atr {

// How the true-range series is smoothed. kRma is Wilder's original
// (alpha = 1/period) and the conventional default.
enum Smoothing { kRma = 0, kSma = 1, kEma = 2 };

// Computes ATR over [candles, candles+n). `period` is clamped to >= 1.
// Fills `out` (resized to n): out[i] is the ATR at candle i in price units, or
// NaN before the first defined value. True range exists from bar 0 (which has
// no previous close, so it falls back to high - low), so the first ATR lands at
// index period - 1 — one earlier than RSI.
void compute(const ::VroomCandle* candles, std::size_t n, int period,
             int smoothing, std::vector<double>& out);

}  // namespace vroom::atr
