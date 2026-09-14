#include "atr.h"

#include <algorithm>  // std::max
#include <cmath>      // std::fabs, std::nan

#include "ma.h"
#include "series_ma.h"

namespace vroom::atr {

void compute(const ::VroomCandle* candles, std::size_t n, int period,
             int smoothing, std::vector<double>& out) {
    out.assign(n, std::nan(""));
    if (!candles || period < 1) return;
    const std::size_t P = static_cast<std::size_t>(period);
    if (n < P) return;

    std::vector<double> tr(n);
    tr[0] = candles[0].high - candles[0].low;  // no previous close to gap from
    for (std::size_t i = 1; i < n; ++i) {
        const double prev_close = candles[i - 1].close;
        tr[i] = std::max({candles[i].high - candles[i].low,
                          std::fabs(candles[i].high - prev_close),
                          std::fabs(candles[i].low - prev_close)});
    }

    if (smoothing == kSma) {
        vroom::series_ma::smooth(tr, vroom::ma::KIND_SMA, period, out);
        return;
    }
    if (smoothing == kEma) {
        vroom::series_ma::smooth(tr, vroom::ma::KIND_EMA, period, out);
        return;
    }

    // Wilder's RMA: seeded with the simple average of the first P true ranges,
    // then avg = (prevAvg * (P - 1) + current) / P.
    double avg = 0.0;
    for (std::size_t i = 0; i < P; ++i) avg += tr[i];
    avg /= static_cast<double>(P);
    out[P - 1] = avg;

    const double pm1 = static_cast<double>(P - 1);
    const double pd = static_cast<double>(P);
    for (std::size_t i = P; i < n; ++i) {
        avg = (avg * pm1 + tr[i]) / pd;
        out[i] = avg;
    }
}

double autoscale(const double* visible, std::size_t n) {
    double scale = 0.0;
    if (!visible) return scale;
    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(visible[i])) scale = std::max(scale, visible[i]);
    }
    return scale;
}

double band_fraction(double v, double scale, double y_scale) {
    // Nothing on show yet — everything sits on the baseline rather than
    // dividing by zero.
    if (!(scale > 0.0)) return 0.0;
    return (v / scale) * kBandPadFraction * y_scale;
}

}  // namespace vroom::atr
