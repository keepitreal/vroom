#include "vwap.h"

#include <cmath>  // std::nan
#include <cstdint>

namespace vroom::vwap {

namespace {
constexpr int64_t kDayMs = 86'400'000;

// Floor division (rounds toward -inf), so a candle before the shifted day start
// buckets into the previous session rather than truncating toward zero.
int64_t floor_div(int64_t a, int64_t b) {
    int64_t q = a / b;
    int64_t r = a % b;
    if (r != 0 && ((r < 0) != (b < 0))) --q;
    return q;
}
}  // namespace

void compute(const ::VroomCandle* candles, std::size_t n, int reset_offset_min,
             std::vector<double>& vwap_out,
             std::vector<unsigned char>& break_out) {
    vwap_out.assign(n, std::nan(""));
    break_out.assign(n, 0);
    if (!candles) return;

    const int64_t offset_ms = static_cast<int64_t>(reset_offset_min) * 60'000;
    double cum_pv = 0.0;
    double cum_v = 0.0;
    int64_t prev_key = 0;
    bool have_prev = false;

    for (std::size_t i = 0; i < n; ++i) {
        const int64_t key = floor_div(candles[i].time_ms - offset_ms, kDayMs);
        if (!have_prev || key != prev_key) {
            cum_pv = 0.0;
            cum_v = 0.0;
            if (have_prev) break_out[i] = 1;  // new session (not the first)
            have_prev = true;
            prev_key = key;
        }
        const double typical =
            (candles[i].high + candles[i].low + candles[i].close) / 3.0;
        cum_pv += typical * candles[i].volume;
        cum_v += candles[i].volume;
        if (cum_v > 0.0) vwap_out[i] = cum_pv / cum_v;
    }
}

}  // namespace vroom::vwap
