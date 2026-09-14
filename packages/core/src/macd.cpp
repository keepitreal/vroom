#include "macd.h"

#include <algorithm>  // std::max
#include <cmath>      // std::nan, std::isfinite, std::abs

#include "ma.h"
#include "series_ma.h"

namespace vroom::macd {

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

    vroom::series_ma::smooth(macd_out, signal_ma_kind, signal, signal_out);

    for (std::size_t i = 0; i < n; ++i) {
        if (std::isfinite(macd_out[i]) && std::isfinite(signal_out[i])) {
            hist_out[i] = macd_out[i] - signal_out[i];
        }
    }
}

double autoscale(const double* macd, const double* signal, const double* hist,
                 std::size_t n) {
    double scale = 0.0;
    const auto track = [&](const double* s) {
        if (!s) return;
        for (std::size_t i = 0; i < n; ++i) {
            if (std::isfinite(s[i])) scale = std::max(scale, std::abs(s[i]));
        }
    };
    track(macd);
    track(signal);
    track(hist);
    return scale;
}

double band_fraction(double v, double scale, double y_scale) {
    // Nothing on show yet — everything collapses onto the zero line rather than
    // dividing by zero.
    if (!(scale > 0.0)) return 0.5;
    return 0.5 + (v / scale) * 0.5 * kBandPadFraction * y_scale;
}

}  // namespace vroom::macd
