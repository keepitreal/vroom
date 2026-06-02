#include "viewport.h"

#include <algorithm>
#include <limits>

namespace vroom {

float candle_body_width(const Layout& layout, size_t count) {
    if (count == 0) return 0.f;
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    const float stride = usable / static_cast<float>(count);
    return stride * layout.candle_width_ratio;
}

float candle_center_x(const Layout& layout, size_t count, size_t i) {
    if (count == 0) return 0.f;
    const float usable =
        layout.width_px - layout.y_axis_width_px - layout.right_padding_px;
    const float stride = usable / static_cast<float>(count);
    return stride * (static_cast<float>(i) + 0.5f);
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
    // The candle drawing area is the full height minus the x-axis strip
    // at the bottom.
    const float candle_area_h = layout.height_px - layout.x_axis_height_px;
    const float top = candle_area_h * layout.top_padding_frac;
    const float bot = candle_area_h * (1.f - layout.bottom_padding_frac);
    const float draw_h = bot - top;
    const double range = bounds.max - bounds.min;
    if (range <= 0.0) return (top + bot) * 0.5f;
    const double t = (price - bounds.min) / range;  // 0..1, 0 at min
    return bot - static_cast<float>(t) * draw_h;    // invert: high price → low y
}

}  // namespace vroom
