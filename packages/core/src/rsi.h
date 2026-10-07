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

// Computes RSI over the closes of [candles, candles+n). `period` must be >= 2
// (otherwise every value is NaN). Fills `out` (resized to n): out[i] is RSI in
// [0,100] at candle i, or NaN where it's undefined:
//   - a candle whose close is <= 0 or non-finite (a backfilled placeholder);
//   - the first `period` candles of each run of priced candles — a run seeds
//     on its own, so the first value lands `period` candles after the run
//     starts (index == period when the series opens priced);
//   - a window whose closes never moved (average gain and loss both zero).
void compute(const ::VroomCandle* candles, std::size_t n, int period,
             std::vector<double>& out);

// Moving average of an RSI series (the "RSI-based MA" / trendline that most RSI
// indicators overlay; crossovers of RSI vs. this line are the common signal).
// `kind` is a vroom::ma KIND_* value. `ma_period` is clamped to >= 1. Fills
// `out` (resized to rsi.size()), NaN until `ma_period` valid RSI values exist —
// either kind produces its first value at the same index. A gap in the RSI
// series leaves the trendline undefined through it; it re-seeds after.
void compute_ma(const std::vector<double>& rsi, int ma_period, int kind,
                std::vector<double>& out);

// Where an RSI value sits in the pane band, as a fraction of its height — 0 at
// the bottom edge, 1 at the top. The fixed 0..100 domain maps about the band
// center so the user's y-axis zoom (`y_scale`, 1 = the default fit) stretches
// symmetrically around 50. Split out of the renderer so the interval-morph
// capture maps its geometry through the same math.
double band_fraction(double v, double y_scale);

// Inverse of band_fraction: the RSI value at fraction `f` of the band height.
// Used by the crosshair to read the pane at the pointer.
double value_at_fraction(double f, double y_scale);

}  // namespace vroom::rsi
