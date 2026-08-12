// Moving averages over an already-computed series — pure computation, no Skia.
//
// Distinct from ma.h, which averages a *price* source off the candles. These
// smooth a derived series that may open with a run of NaN warmup (the MACD
// difference, an RSI line), so they skip that run and start counting from the
// first finite value. Both smoothers therefore produce their first value at the
// same index, which is what lets the MA kind be a config toggle.

#pragma once

#include <vector>

namespace vroom::series_ma {

// SMA-seeded EMA over `src`. Fills `out` (resized to src.size()): NaN until the
// seed index, the seed = SMA of the first `period` finite values, then
// ema = alpha*src + (1-alpha)*prev.
void ema_seeded(const std::vector<double>& src, int period,
                std::vector<double>& out);

// Rolling SMA over `src`, skipping the same leading NaN run as ema_seeded.
void sma_seeded(const std::vector<double>& src, int period,
                std::vector<double>& out);

// Dispatches on a vroom::ma KIND_* value: KIND_SMA picks sma_seeded, anything
// else ema_seeded.
void smooth(const std::vector<double>& src, int kind, int period,
            std::vector<double>& out);

}  // namespace vroom::series_ma
