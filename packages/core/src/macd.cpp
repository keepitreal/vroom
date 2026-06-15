#include "macd.h"

#include <cmath>  // std::nan, std::isfinite

namespace vroom::macd {

namespace {
// SMA-seeded EMA over `src` (which may have a leading NaN run). Fills `out`
// (resized to src.size()): NaN until the seed index, the seed = SMA of the
// first `period` finite values, then ema = alpha*src + (1-alpha)*prev.
void ema_seeded(const std::vector<double>& src, int period,
                std::vector<double>& out) {
    const std::size_t n = src.size();
    out.assign(n, std::nan(""));
    if (period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);

    std::size_t f = 0;
    while (f < n && !std::isfinite(src[f])) ++f;
    if (f >= n || f + P > n) return;  // not enough finite values to seed

    const std::size_t seed = f + P - 1;
    double sum = 0.0;
    for (std::size_t k = f; k <= seed; ++k) sum += src[k];
    double prev = sum / static_cast<double>(P);
    out[seed] = prev;

    const double alpha = 2.0 / (static_cast<double>(P) + 1.0);
    for (std::size_t i = seed + 1; i < n; ++i) {
        if (!std::isfinite(src[i])) break;  // series are contiguous after f
        prev = alpha * src[i] + (1.0 - alpha) * prev;
        out[i] = prev;
    }
}
}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int fast, int slow,
             int signal, std::vector<double>& macd_out,
             std::vector<double>& signal_out, std::vector<double>& hist_out) {
    macd_out.assign(n, std::nan(""));
    signal_out.assign(n, std::nan(""));
    hist_out.assign(n, std::nan(""));
    if (!candles || fast < 1 || slow < 1 || signal < 1) return;

    std::vector<double> closes(n);
    for (std::size_t i = 0; i < n; ++i) closes[i] = candles[i].close;

    std::vector<double> ema_fast;
    std::vector<double> ema_slow;
    ema_seeded(closes, fast, ema_fast);
    ema_seeded(closes, slow, ema_slow);

    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(ema_fast[i]) && std::isfinite(ema_slow[i])) {
            macd_out[i] = ema_fast[i] - ema_slow[i];
        }
    }

    ema_seeded(macd_out, signal, signal_out);

    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(macd_out[i]) && std::isfinite(signal_out[i])) {
            hist_out[i] = macd_out[i] - signal_out[i];
        }
    }
}

}  // namespace vroom::macd
