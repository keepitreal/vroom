// Moving averages (SMA / EMA) over a candle source series — pure, no Skia, so
// it builds into the unit-test target. Used by the price-pane overlay lines.

#pragma once

#include <cstddef>
#include <vector>

#include "vroom/vroom_chart.h"  // ::VroomCandle

namespace vroom::ma {

// Source selector. Index order is mirrored on the TS side (MA_SOURCES).
enum Source {
    SOURCE_CLOSE = 0,
    SOURCE_OPEN,
    SOURCE_HIGH,
    SOURCE_LOW,
    SOURCE_HL2,    // (high + low) / 2
    SOURCE_HLC3,   // (high + low + close) / 3
    SOURCE_OHLC4,  // (open + high + low + close) / 4
};

// Kind selector (mirrored on the TS side).
enum Kind {
    KIND_SMA = 0,
    KIND_EMA = 1,
};

// The source price for a candle.
double source_value(const ::VroomCandle& c, int source);

// Computes the SMA (kind 0) or SMA-seeded EMA (kind 1) of the chosen source over
// [candles, candles+n). `period` is clamped to >= 1. Fills `out` (resized to n):
// out[i] is the average at candle i, or NaN for i < period-1 and when n < period.
void compute(const ::VroomCandle* candles, std::size_t n, int kind, int period,
             int source, std::vector<double>& out);

}  // namespace vroom::ma
