#include "viewport.h"

#include <algorithm>
#include <limits>

namespace vroom {

float candle_body_width(const Layout& layout,
                        int64_t window_ms,
                        int64_t candle_duration_ms) {
    if (window_ms <= 0 || candle_duration_ms <= 0) return 0.f;
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    const double slot = static_cast<double>(usable) *
        (static_cast<double>(candle_duration_ms) /
         static_cast<double>(window_ms));
    return static_cast<float>(slot * layout.candle_width_ratio);
}

float candle_center_x(const Layout& layout,
                      int64_t time_ms,
                      int64_t candle_duration_ms,
                      int64_t visible_start_ms,
                      int64_t window_ms) {
    if (window_ms <= 0) return 0.f;
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    const double center_time =
        static_cast<double>(time_ms) +
        static_cast<double>(candle_duration_ms) * 0.5;
    const double frac =
        (center_time - static_cast<double>(visible_start_ms)) /
        static_cast<double>(window_ms);
    return static_cast<float>(static_cast<double>(usable) * frac);
}

size_t snap_index_to_candle(const Layout& layout,
                            const ::VroomCandle* candles,
                            size_t count,
                            int64_t candle_duration_ms,
                            int64_t visible_start_ms,
                            int64_t window_ms,
                            float x_px) {
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f) return 0;

    // Pixel x -> the period start time of the candle that would sit there.
    const double target_center =
        static_cast<double>(visible_start_ms) +
        (static_cast<double>(x_px) / static_cast<double>(usable)) *
            static_cast<double>(window_ms);
    const int64_t key =
        static_cast<int64_t>(target_center) - candle_duration_ms / 2;

    // First candle with time_ms >= key, then pick whichever of it / its
    // predecessor is closer in time.
    auto* it = std::lower_bound(
        candles, candles + count, key,
        [](const ::VroomCandle& c, int64_t t) { return c.time_ms < t; });
    if (it == candles) return 0;
    if (it == candles + count) return count - 1;
    const int64_t hi = it->time_ms;
    const int64_t lo = (it - 1)->time_ms;
    return (hi - key < key - lo) ? static_cast<size_t>(it - candles)
                                 : static_cast<size_t>(it - 1 - candles);
}

float snap_x_to_candle(const Layout& layout,
                       const ::VroomCandle* candles,
                       size_t count,
                       int64_t candle_duration_ms,
                       int64_t visible_start_ms,
                       int64_t window_ms,
                       float x_px) {
    if (count == 0 || window_ms <= 0) return x_px;

    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    if (usable <= 0.f) return x_px;

    const size_t idx = snap_index_to_candle(
        layout, candles, count, candle_duration_ms, visible_start_ms,
        window_ms, x_px);
    return candle_center_x(layout, candles[idx].time_ms, candle_duration_ms,
                           visible_start_ms, window_ms);
}

IndexRange visible_indices(const ::VroomCandle* candles,
                           size_t count,
                           int64_t start_ms,
                           int64_t end_ms) {
    if (count == 0) return {0, 0};
    if (start_ms == 0 && end_ms == 0) return {0, count};

    // first index with time_ms >= start_ms
    auto* first = std::lower_bound(
        candles, candles + count, start_ms,
        [](const ::VroomCandle& c, int64_t t) { return c.time_ms < t; });
    // first index with time_ms > end_ms
    auto* last = std::upper_bound(
        candles, candles + count, end_ms,
        [](int64_t t, const ::VroomCandle& c) { return t < c.time_ms; });
    return {
        static_cast<size_t>(first - candles),
        static_cast<size_t>(last - candles),
    };
}

PriceBounds price_bounds(const ::VroomCandle* candles, size_t count) {
    PriceBounds b{
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    };
    for (size_t i = 0; i < count; ++i) {
        b.min = std::min(b.min, candles[i].low);
        b.max = std::max(b.max, candles[i].high);
    }
    if (count == 0) {
        b.min = 0.0;
        b.max = 1.0;
    }
    return b;
}

float price_to_y(const Layout& layout,
                 const PriceBounds& bounds,
                 double price) {
    // The candle drawing area is the full height minus the x-axis strip and
    // any below-chart indicator pane.
    const float candle_area_h = price_pane_bottom(layout);
    const float top = candle_area_h * layout.top_padding_frac;
    const float bot = candle_area_h * (1.f - layout.bottom_padding_frac);
    const float draw_h = bot - top;
    const double range = bounds.max - bounds.min;
    if (range <= 0.0) return (top + bot) * 0.5f;
    const double t = (price - bounds.min) / range;  // 0..1, 0 at min
    return bot - static_cast<float>(t) * draw_h;    // invert: high price → low y
}

}  // namespace vroom
