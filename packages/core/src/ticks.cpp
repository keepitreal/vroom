#include "ticks.h"

#include <algorithm>
#include <cmath>

namespace vroom {

int64_t pick_time_interval(int64_t window_ms, float candle_area_w) {
    static constexpr int64_t kIntervals[] = {
        60'000LL,                // 1m
        5 * 60'000LL,            // 5m
        15 * 60'000LL,           // 15m
        30 * 60'000LL,           // 30m
        60 * 60'000LL,           // 1h
        2 * 60 * 60'000LL,       // 2h
        4 * 60 * 60'000LL,       // 4h
        6 * 60 * 60'000LL,       // 6h
        12 * 60 * 60'000LL,      // 12h
        24 * 60 * 60'000LL,      // 1d
        2 * 24 * 60 * 60'000LL,  // 2d
        7 * 24 * 60 * 60'000LL,  // 1w
    };
    if (window_ms <= 0 || candle_area_w <= 0.f) return kIntervals[0];
    const double ms_per_px =
        static_cast<double>(window_ms) / candle_area_w;
    for (int64_t i : kIntervals) {
        if (static_cast<double>(i) / ms_per_px >= kXLabelMinSpacing) {
            return i;
        }
    }
    return kIntervals[sizeof(kIntervals) / sizeof(kIntervals[0]) - 1];
}

double pick_price_interval(double range, float candle_area_h) {
    if (range <= 0.0 || candle_area_h <= 0.f) return 1.0;
    const int target_count =
        std::max(2, static_cast<int>(candle_area_h / kYLabelTargetSpacing));
    const double rough = range / target_count;
    const double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
    const double normalized = rough / magnitude;
    double nice;
    if (normalized < 1.5) nice = 1.0;
    else if (normalized < 3.5) nice = 2.0;
    else if (normalized < 7.5) nice = 5.0;
    else nice = 10.0;
    return nice * magnitude;
}

}  // namespace vroom
