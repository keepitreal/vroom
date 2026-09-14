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

// How much of the pane's half-band an extreme is allowed to fill, so a peak
// stops short of the separator instead of touching it.
constexpr double kBandPadFraction = 0.85;

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

// The amplitude the pane band is fitted to, symmetric about zero: the largest
// finite magnitude across the series on show, or 0 when there is nothing to
// plot. Pass nullptr for a hidden series so the rest fill the band.
double autoscale(const double* macd, const double* signal, const double* hist,
                 std::size_t n);

// Where a value sits in the pane band, as a fraction of its height — 0 at the
// bottom edge, 1 at the top, 0.5 on the zero line. `y_scale` is the user's
// y-axis zoom (1 = the default fit). Split out of the renderer so the
// interval-morph capture maps its geometry through the same math.
double band_fraction(double v, double scale, double y_scale);

}  // namespace vroom::macd
