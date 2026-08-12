// MACD — pure computation, no Skia. Kept Skia-free so it builds into the
// unit-test target. Mirrors rsi.cpp's NaN-warmup convention.
//
// MACD line   = MA(fast) - MA(slow) of the chosen price source
// Signal line = MA(signal) of the MACD line
// Histogram   = MACD - Signal
// EMAs are SMA-seeded: NaN until the first `period` values exist (the seed is
// their simple average), then ema = alpha*src + (1-alpha)*prev, alpha=2/(p+1).

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::macd {

// Computes MACD over [candles, candles+n). fast/slow/signal are clamped to >= 1
// (callers pass 12/26/9 by default; slow should exceed fast). `source` selects
// the price series (vroom::ma::Source); `ma_kind` picks the averaging for the
// fast/slow legs and `signal_ma_kind` for the signal line (vroom::ma::Kind).
// Fills three series (each resized to n, NaN where undefined):
//   macd_out   — MA(fast) - MA(slow), defined from index slow-1
//   signal_out — MA(signal) of macd_out, defined from (slow-1)+(signal-1)
//   hist_out   — macd_out - signal_out, defined where both are
void compute(const ::VroomCandle* candles, std::size_t n, int fast, int slow,
             int signal, int source, int ma_kind, int signal_ma_kind,
             std::vector<double>& macd_out, std::vector<double>& signal_out,
             std::vector<double>& hist_out);

}  // namespace vroom::macd
