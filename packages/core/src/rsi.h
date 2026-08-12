// Wilder's RSI — pure computation, no Skia. Kept Skia-free so it builds into
// the unit-test target.
//
// RSI = 100 - 100/(1+RS), RS = avgGain/avgLoss over `period` close-to-close
// changes. Seed = simple average of the first `period` gains/losses; subsequent
// values use Wilder smoothing avg = (prevAvg*(period-1) + current)/period.

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::rsi {

// Computes RSI over the closes of [candles, candles+n). `period` is clamped to
// >= 2. Fills `out` (resized to n): out[i] is RSI in [0,100] at candle i, or
// NaN for i < period and for every index when n <= period (insufficient data).
// The first defined value is at index == period (the (period+1)-th candle).
void compute(const ::VroomCandle* candles, std::size_t n, int period,
             std::vector<double>& out);

// Moving average of an RSI series (the "RSI-based MA" / trendline that most RSI
// indicators overlay; crossovers of RSI vs. this line are the common signal).
// `kind` is a vroom::ma KIND_* value. `ma_period` is clamped to >= 1. Fills
// `out` (resized to rsi.size()), NaN until `ma_period` valid RSI values exist —
// either kind produces its first value at the same index.
void compute_ma(const std::vector<double>& rsi, int ma_period, int kind,
                std::vector<double>& out);

}  // namespace vroom::rsi
