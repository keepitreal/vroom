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

// How much of the pane band the curve is allowed to fill, so a peak stops short
// of the separator above it instead of touching.
constexpr double kBandPadFraction = 0.85;

// Computes ATR over [candles, candles+n). `period` is clamped to >= 1.
// Fills `out` (resized to n): out[i] is the ATR at candle i in price units, or
// NaN before the first defined value. True range exists from bar 0 (which has
// no previous close, so it falls back to high - low), so the first ATR lands at
// index period - 1 — one earlier than RSI.
void compute(const ::VroomCandle* candles, std::size_t n, int period,
             int smoothing, std::vector<double>& out);

// The value the pane band is fitted to: the largest finite ATR on show, or 0
// when there is nothing to plot. ATR is strictly positive, so the domain runs
// from 0 at the band's bottom edge up to this.
double autoscale(const double* visible, std::size_t n);

// Where a value sits in the pane band, as a fraction of its height — 0 at the
// bottom edge, 1 at the top. `y_scale` is the user's y-axis zoom (1 = the
// default fit). Split out of the renderer so the interval-morph capture maps
// its geometry through the same math the pane draws with.
double band_fraction(double v, double scale, double y_scale);

}  // namespace vroom::atr
