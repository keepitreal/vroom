#include "ichimoku.h"

#include <algorithm>  // std::max, std::min
#include <cmath>      // std::nan

namespace vroom::ichimoku {
namespace {

// Midpoint of the high/low range over each trailing `period`-bar window — the
// shape all three of Ichimoku's averaged lines share. NaN over the warmup.
void midpoint_series(const ::VroomCandle* candles, std::size_t n, int period,
                     std::vector<double>& out) {
    out.assign(n, std::nan(""));
    if (!candles || period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);
    if (n < P) return;

    for (std::size_t i = P - 1; i < n; ++i) {
        const std::size_t s = i + 1 - P;
        double hi = candles[s].high;
        double lo = candles[s].low;
        for (std::size_t j = s + 1; j <= i; ++j) {
            hi = std::max(hi, candles[j].high);
            lo = std::min(lo, candles[j].low);
        }
        out[i] = (hi + lo) * 0.5;
    }
}

}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int tenkan_period,
             int kijun_period, int senkou_b_period,
             std::vector<double>& tenkan, std::vector<double>& kijun,
             std::vector<double>& senkou_a, std::vector<double>& senkou_b,
             std::vector<double>& chikou) {
    midpoint_series(candles, n, tenkan_period, tenkan);
    midpoint_series(candles, n, kijun_period, kijun);
    midpoint_series(candles, n, senkou_b_period, senkou_b);

    // NaN propagates through the average, so span A is only defined once both
    // of its inputs are — no separate warmup bound to track.
    senkou_a.assign(n, std::nan(""));
    for (std::size_t i = 0; i < n; ++i) {
        senkou_a[i] = (tenkan[i] + kijun[i]) * 0.5;
    }

    chikou.assign(n, std::nan(""));
    if (!candles) return;
    for (std::size_t i = 0; i < n; ++i) chikou[i] = candles[i].close;
}

IndexRange shifted_source_range(const ::VroomCandle* candles, std::size_t n,
                                int64_t start_ms, int64_t end_ms,
                                int64_t shift_ms) {
    // 0/0 is visible_indices' "everything" sentinel (an unframed viewport).
    // Shifting it would turn that into an empty range, so pass it through.
    if (start_ms == 0 && end_ms == 0) {
        return vroom::visible_indices(candles, n, 0, 0);
    }
    return vroom::visible_indices(candles, n, start_ms - shift_ms,
                                  end_ms - shift_ms);
}

}  // namespace vroom::ichimoku
