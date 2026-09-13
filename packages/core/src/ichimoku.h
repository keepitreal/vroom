// Ichimoku Kinko Hyo over a candle series — pure, no Skia, so it builds into
// the unit-test target. Drawn as five price-pane overlay lines plus the cloud
// shaded between the two leading spans.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "viewport.h"           // vroom::IndexRange
#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::ichimoku {

// Computes the five Ichimoku series over [candles, candles+n). Each period is
// clamped to >= 1.
//
//   tenkan   = midpoint of the high/low range over the trailing tenkan window
//   kijun    = same over the trailing kijun window
//   senkou_a = (tenkan + kijun) / 2
//   senkou_b = midpoint of the high/low range over the trailing senkou_b window
//   chikou   = close
//
// Every output is resized to n and indexed by the bar it was computed from —
// displacement is a drawing concern, so senkou_a/senkou_b/chikou are *not*
// shifted here. Values are NaN over each series' warmup and when n is shorter
// than the window a series needs.
void compute(const ::VroomCandle* candles, std::size_t n, int tenkan_period,
             int kijun_period, int senkou_b_period,
             std::vector<double>& tenkan, std::vector<double>& kijun,
             std::vector<double>& senkou_a, std::vector<double>& senkou_b,
             std::vector<double>& chikou);

// Source indices whose *shifted* plot time lands in [start_ms, end_ms], for a
// series drawn at `candles[i].time_ms + shift_ms`.
//
// Shifting the query window back by the same amount is what lets the leading
// spans run past the newest candle: the range is found among bars that exist,
// while the x each one draws at does not have to.
IndexRange shifted_source_range(const ::VroomCandle* candles, std::size_t n,
                                int64_t start_ms, int64_t end_ms,
                                int64_t shift_ms);

}  // namespace vroom::ichimoku
