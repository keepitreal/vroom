#include "rsi.h"

#include <cmath>  // std::nan, std::isfinite

#include "series_ma.h"

namespace vroom::rsi {

namespace {
// Standard rule: avgLoss == 0 → 100 (no downside). When avgGain == 0 and
// avgLoss > 0, RS = 0 → RSI = 0 falls out naturally. Both zero means the
// window never moved, which says nothing about momentum, so it's undefined
// rather than a pinned 100.
double rsi_from(double avg_gain, double avg_loss) {
    if (avg_gain == 0.0 && avg_loss == 0.0) return std::nan("");
    if (avg_loss == 0.0) return 100.0;
    const double rs = avg_gain / avg_loss;
    return 100.0 - 100.0 / (1.0 + rs);
}

// Consumers backfill candles before a token's launch (or across outages) with
// zero prices. No traded asset closes at or below zero, so those are missing
// data, not a price the first real candle rallied from.
bool has_price(const ::VroomCandle& c) {
    return std::isfinite(c.close) && c.close > 0.0;
}
}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int period,
             std::vector<double>& out) {
    out.assign(n, std::nan(""));
    if (!candles || period < 2) return;
    const std::size_t P = static_cast<std::size_t>(period);
    const double pm1 = static_cast<double>(P - 1);
    const double pd = static_cast<double>(P);

    // Each run of priced candles seeds on its own: the simple average of its
    // first P gains/losses, then Wilder smoothing. A missing candle ends the run.
    std::size_t deltas = 0;  // deltas taken in the current run
    double avg_gain = 0.0;
    double avg_loss = 0.0;
    bool in_run = false;
    for (std::size_t i = 0; i < n; ++i) {
        if (!has_price(candles[i])) {
            in_run = false;
            continue;
        }
        if (!in_run) {
            in_run = true;
            deltas = 0;
            avg_gain = 0.0;
            avg_loss = 0.0;
            continue;
        }
        const double d = candles[i].close - candles[i - 1].close;
        const double gain = d > 0.0 ? d : 0.0;
        const double loss = d < 0.0 ? -d : 0.0;
        ++deltas;
        if (deltas < P) {
            avg_gain += gain;
            avg_loss += loss;
            continue;
        }
        if (deltas == P) {
            avg_gain = (avg_gain + gain) / pd;
            avg_loss = (avg_loss + loss) / pd;
        } else {
            avg_gain = (avg_gain * pm1 + gain) / pd;
            avg_loss = (avg_loss * pm1 + loss) / pd;
        }
        out[i] = rsi_from(avg_gain, avg_loss);
    }
}

void compute_ma(const std::vector<double>& rsi, int ma_period, int kind,
                std::vector<double>& out) {
    vroom::series_ma::smooth(rsi, kind, ma_period, out);
}

double band_fraction(double v, double y_scale) {
    return 0.5 + ((v - 50.0) / 100.0) * y_scale;
}

double value_at_fraction(double f, double y_scale) {
    if (!(y_scale > 0.0)) return std::nan("");
    return 50.0 + (f - 0.5) * 100.0 / y_scale;
}

}  // namespace vroom::rsi
