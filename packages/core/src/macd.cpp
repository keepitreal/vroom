#include "macd.h"

#include <cmath>  // std::nan, std::isfinite

#include "ma.h"

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

// Rolling SMA over `src`, skipping the same leading NaN run as ema_seeded so
// both smoothers start on the same index.
void sma_seeded(const std::vector<double>& src, int period,
                std::vector<double>& out) {
    const std::size_t n = src.size();
    out.assign(n, std::nan(""));
    if (period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);

    std::size_t f = 0;
    while (f < n && !std::isfinite(src[f])) ++f;
    if (f >= n || f + P > n) return;

    double sum = 0.0;
    for (std::size_t i = f; i < n; ++i) {
        if (!std::isfinite(src[i])) break;  // series are contiguous after f
        sum += src[i];
        if (i >= f + P) sum -= src[i - P];
        if (i >= f + P - 1) out[i] = sum / static_cast<double>(P);
    }
}

void smooth_series(const std::vector<double>& src, int kind, int period,
                   std::vector<double>& out) {
    if (kind == vroom::ma::KIND_SMA) {
        sma_seeded(src, period, out);
    } else {
        ema_seeded(src, period, out);
    }
}
}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int fast, int slow,
             int signal, int source, int ma_kind, int signal_ma_kind,
             std::vector<double>& macd_out, std::vector<double>& signal_out,
             std::vector<double>& hist_out) {
    macd_out.assign(n, std::nan(""));
    signal_out.assign(n, std::nan(""));
    hist_out.assign(n, std::nan(""));
    if (!candles || fast < 1 || slow < 1 || signal < 1) return;

    std::vector<double> ma_fast;
    std::vector<double> ma_slow;
    vroom::ma::compute(candles, n, ma_kind, fast, source, ma_fast);
    vroom::ma::compute(candles, n, ma_kind, slow, source, ma_slow);

    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(ma_fast[i]) && std::isfinite(ma_slow[i])) {
            macd_out[i] = ma_fast[i] - ma_slow[i];
        }
    }

    smooth_series(macd_out, signal_ma_kind, signal, signal_out);

    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(macd_out[i]) && std::isfinite(signal_out[i])) {
            hist_out[i] = macd_out[i] - signal_out[i];
        }
    }
}

}  // namespace vroom::macd
