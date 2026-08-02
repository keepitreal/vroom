#include "bollinger.h"

#include <cmath>  // std::nan, std::sqrt

#include "ma.h"

namespace vroom::bollinger {

void compute(const ::VroomCandle* candles, std::size_t n, int period,
             double mult, int source, int basis_kind,
             std::vector<double>& middle, std::vector<double>& upper,
             std::vector<double>& lower) {
    vroom::ma::compute(candles, n, basis_kind, period, source, middle);
    upper.assign(n, std::nan(""));
    lower.assign(n, std::nan(""));
    if (!candles || period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);
    if (n < P) return;

    std::vector<double> src(n);
    for (std::size_t i = 0; i < n; ++i) src[i] = vroom::ma::source_value(candles[i], source);

    // Two-pass stdev around the true window mean per bar. O(n·period), but
    // numerically stable — the rolling Σx²−n·mean² form cancels catastrophically
    // on large prices with small deviations (e.g. BTC-scale values).
    for (std::size_t i = P - 1; i < n; ++i) {
        const std::size_t s = i + 1 - P;
        double mean = 0.0;
        for (std::size_t j = s; j <= i; ++j) mean += src[j];
        mean /= static_cast<double>(P);
        double var = 0.0;
        for (std::size_t j = s; j <= i; ++j) {
            const double d = src[j] - mean;
            var += d * d;
        }
        var /= static_cast<double>(P);
        const double band = mult * std::sqrt(var);
        upper[i] = middle[i] + band;
        lower[i] = middle[i] - band;
    }
}

}  // namespace vroom::bollinger
