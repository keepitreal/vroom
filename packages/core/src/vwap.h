// VWAP (Volume-Weighted Average Price) — pure, no Skia, so it builds into the
// unit-test target. Session anchor only: cumulative sums reset each UTC day
// shifted by a configurable offset.
//
//   VWAP = sum(typical*volume) / sum(volume)   over the current session
//   typical = (high + low + close) / 3   (hlc3)

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::vwap {

// Computes session VWAP over [candles, candles+n). The session boundary is the
// UTC day shifted by `reset_offset_min` minutes (e.g. 60 -> sessions start at
// 01:00 UTC). Fills:
//   vwap_out  (resized to n): VWAP at candle i, NaN where cumulative volume is 0
//   break_out (resized to n): 1 at the first candle of each new session after
//             the first (so the renderer lifts the pen across resets), else 0.
void compute(const ::VroomCandle* candles, std::size_t n, int reset_offset_min,
             std::vector<double>& vwap_out,
             std::vector<unsigned char>& break_out);

}  // namespace vroom::vwap
