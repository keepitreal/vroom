#include "ticks.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>

namespace vroom {

namespace {

// Approximate average lengths used only to choose a cadence (the actual ticks
// land on exact calendar boundaries). 30.436875 d / 365.2425 d.
constexpr int64_t kMonthMs = 2'629'746'000LL;
constexpr int64_t kYearMs = 31'556'952'000LL;

// Smallest 1/2/5 × 10ⁿ integer that is >= x (>= 1).
int nice_ceil_int(double x) {
    if (x <= 1.0) return 1;
    const double mag = std::pow(10.0, std::floor(std::log10(x)));
    for (double m : {1.0, 2.0, 5.0, 10.0}) {
        const double cand = m * mag;
        if (cand >= x) return static_cast<int>(cand);
    }
    return static_cast<int>(10.0 * mag);
}

// Local-time midnight of year/mon(0-11)/mday as epoch ms.
int64_t ymd_to_ms(int year, int mon, int mday) {
    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = mon;
    t.tm_mday = mday;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;
    return static_cast<int64_t>(std::mktime(&t)) * 1000;
}

}  // namespace

TimeTick pick_time_tick(int64_t window_ms, float candle_area_w) {
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
    if (window_ms <= 0 || candle_area_w <= 0.f) {
        return {TimeUnit::Fixed, kIntervals[0], 0};
    }
    const double ms_per_px = static_cast<double>(window_ms) / candle_area_w;

    for (int64_t i : kIntervals) {
        if (static_cast<double>(i) / ms_per_px >= kXLabelMinSpacing) {
            return {TimeUnit::Fixed, i, 0};
        }
    }

    static constexpr int kMonthSteps[] = {1, 3, 6};
    for (int s : kMonthSteps) {
        const double span = static_cast<double>(s) *
                            static_cast<double>(kMonthMs);
        if (span / ms_per_px >= kXLabelMinSpacing) {
            return {TimeUnit::Month, 0, s};
        }
    }

    // Years: smallest nice step whose spacing clears the minimum. Unbounded, so
    // arbitrarily wide windows still get non-overlapping labels.
    const double needed = (static_cast<double>(kXLabelMinSpacing) * ms_per_px) /
                          static_cast<double>(kYearMs);
    return {TimeUnit::Year, 0, nice_ceil_int(needed)};
}

int64_t first_tick_at_or_after(int64_t from_ms, const TimeTick& tick) {
    if (tick.unit == TimeUnit::Fixed) {
        const int64_t step = tick.step_ms;
        if (step <= 0) return from_ms;
        int64_t t = (from_ms / step) * step;
        if (t < from_ms) t += step;
        return t;
    }

    const time_t from_s = static_cast<time_t>(from_ms / 1000);
    std::tm lt{};
    localtime_r(&from_s, &lt);
    const int step = tick.step > 0 ? tick.step : 1;

    if (tick.unit == TimeUnit::Year) {
        int year = ((lt.tm_year + 1900 + step - 1) / step) * step;
        for (;;) {
            const int64_t ms = ymd_to_ms(year, 0, 1);
            if (ms >= from_ms) return ms;
            year += step;
        }
    }

    // Month: align the month index (year*12 + mon) up to a multiple of step.
    int mi = (((lt.tm_year + 1900) * 12 + lt.tm_mon) + step - 1) / step * step;
    for (;;) {
        const int64_t ms = ymd_to_ms(mi / 12, mi % 12, 1);
        if (ms >= from_ms) return ms;
        mi += step;
    }
}

int64_t next_tick(int64_t t, const TimeTick& tick) {
    if (tick.unit == TimeUnit::Fixed) {
        return t + (tick.step_ms > 0 ? tick.step_ms : 0);
    }

    const time_t s = static_cast<time_t>(t / 1000);
    std::tm lt{};
    localtime_r(&s, &lt);
    const int step = tick.step > 0 ? tick.step : 1;

    if (tick.unit == TimeUnit::Year) {
        return ymd_to_ms(lt.tm_year + 1900 + step, 0, 1);
    }
    const int mi = (lt.tm_year + 1900) * 12 + lt.tm_mon + step;
    return ymd_to_ms(mi / 12, mi % 12, 1);
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

int price_decimals(double interval) {
    if (!(interval > 0.0) || !std::isfinite(interval)) return 2;
    const double d = std::ceil(-std::log10(interval) - 1e-12);
    if (d < 0.0) return 0;
    if (d > 12.0) return 12;
    return static_cast<int>(d);
}

void format_price(char* buf, size_t buf_size, double price, int decimals) {
    if (!buf || buf_size == 0) return;
    std::snprintf(buf, buf_size, "%.*f", decimals, price);
}

}  // namespace vroom
