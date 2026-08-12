#include "series_ma.h"

#include <cmath>  // std::nan, std::isfinite
#include <cstddef>

#include "ma.h"

namespace vroom::series_ma {

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

void smooth(const std::vector<double>& src, int kind, int period,
            std::vector<double>& out) {
    if (kind == vroom::ma::KIND_SMA) {
        sma_seeded(src, period, out);
    } else {
        ema_seeded(src, period, out);
    }
}

}  // namespace vroom::series_ma
