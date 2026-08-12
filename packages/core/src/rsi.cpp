#include "rsi.h"

#include <cmath>  // std::nan, std::isfinite

#include "series_ma.h"

namespace vroom::rsi {

namespace {
// Standard rule: avgLoss == 0 → 100 (no downside). When avgGain == 0 and
// avgLoss > 0, RS = 0 → RSI = 0 falls out naturally.
double rsi_from(double avg_gain, double avg_loss) {
    if (avg_loss == 0.0) return 100.0;
    const double rs = avg_gain / avg_loss;
    return 100.0 - 100.0 / (1.0 + rs);
}
}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int period,
             std::vector<double>& out) {
    out.assign(n, std::nan(""));
    if (!candles || period < 2) return;
    const std::size_t P = static_cast<std::size_t>(period);
    if (n <= P) return;  // need `period` deltas to seed; first value at index P

    // Seed: simple average of the first P gains/losses (deltas 1..P).
    double avg_gain = 0.0;
    double avg_loss = 0.0;
    for (std::size_t i = 1; i <= P; ++i) {
        const double d = candles[i].close - candles[i - 1].close;
        if (d >= 0.0) avg_gain += d;
        else avg_loss += -d;
    }
    avg_gain /= static_cast<double>(P);
    avg_loss /= static_cast<double>(P);
    out[P] = rsi_from(avg_gain, avg_loss);

    // Wilder smoothing for the remaining candles.
    const double pm1 = static_cast<double>(P - 1);
    const double pd = static_cast<double>(P);
    for (std::size_t i = P + 1; i < n; ++i) {
        const double d = candles[i].close - candles[i - 1].close;
        const double gain = d > 0.0 ? d : 0.0;
        const double loss = d < 0.0 ? -d : 0.0;
        avg_gain = (avg_gain * pm1 + gain) / pd;
        avg_loss = (avg_loss * pm1 + loss) / pd;
        out[i] = rsi_from(avg_gain, avg_loss);
    }
}

void compute_ma(const std::vector<double>& rsi, int ma_period, int kind,
                std::vector<double>& out) {
    vroom::series_ma::smooth(rsi, kind, ma_period, out);
}

}  // namespace vroom::rsi
