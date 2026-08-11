// Bollinger Bands over a candle source series — pure, no Skia, so it builds
// into the unit-test target. Drawn as price-pane overlay lines + a band fill.

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::bollinger {

// Computes the three band series over [candles, candles+n). `period` is clamped
// to >= 1; `mult` is the standard-deviation multiplier. `source` is a
// vroom::ma::Source index and `basis_kind` a vroom::ma::Kind (SMA/EMA).
//
//   middle = ma::compute(basis_kind, period, source)
//   upper/lower = middle ± mult * population stdev of source over the trailing
//                 period window
//
// The stdev always uses the window's arithmetic mean, even when basis_kind
// selects an EMA basis line (the standard semantics).
//
// Each output is resized to n; values are NaN for i < period-1 and when
// n < period.
void compute(const ::VroomCandle* candles, std::size_t n, int period,
             double mult, int source, int basis_kind,
             std::vector<double>& middle, std::vector<double>& upper,
             std::vector<double>& lower);

}  // namespace vroom::bollinger
