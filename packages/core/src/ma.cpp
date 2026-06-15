#include "ma.h"

#include <cmath>  // std::nan

namespace vroom::ma {

double source_value(const ::VroomCandle& c, int source) {
    switch (source) {
        case SOURCE_OPEN: return c.open;
        case SOURCE_HIGH: return c.high;
        case SOURCE_LOW: return c.low;
        case SOURCE_HL2: return (c.high + c.low) / 2.0;
        case SOURCE_HLC3: return (c.high + c.low + c.close) / 3.0;
        case SOURCE_OHLC4: return (c.open + c.high + c.low + c.close) / 4.0;
        case SOURCE_CLOSE:
        default: return c.close;
    }
}

void compute(const ::VroomCandle* candles, std::size_t n, int kind, int period,
             int source, std::vector<double>& out) {
    out.assign(n, std::nan(""));
    if (!candles || period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);
    if (n < P) return;

    std::vector<double> src(n);
    for (std::size_t i = 0; i < n; ++i) src[i] = source_value(candles[i], source);

    if (kind == KIND_EMA) {
        // SMA-seeded EMA: seed at index P-1 with the SMA of the first P values.
        double sum = 0.0;
        for (std::size_t i = 0; i < P; ++i) sum += src[i];
        double prev = sum / static_cast<double>(P);
        out[P - 1] = prev;
        const double alpha = 2.0 / (static_cast<double>(P) + 1.0);
        for (std::size_t i = P; i < n; ++i) {
            prev = alpha * src[i] + (1.0 - alpha) * prev;
            out[i] = prev;
        }
        return;
    }

    // SMA via a rolling window sum (O(n)).
    double sum = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        sum += src[i];
        if (i >= P) sum -= src[i - P];
        if (i >= P - 1) out[i] = sum / static_cast<double>(P);
    }
}

}  // namespace vroom::ma
