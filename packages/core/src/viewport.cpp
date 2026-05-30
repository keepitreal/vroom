#include "viewport.h"

#include <algorithm>
#include <limits>

namespace vroom {

float candle_body_width(const Layout& layout, size_t count) {
    if (count == 0) return 0.f;
    const float usable = layout.width_px - layout.right_padding_px;
    const float stride = usable / static_cast<float>(count);
    return stride * layout.candle_width_ratio;
}

float candle_center_x(const Layout& layout, size_t count, size_t i) {
    if (count == 0) return 0.f;
    const float usable = layout.width_px - layout.right_padding_px;
    const float stride = usable / static_cast<float>(count);
    return stride * (static_cast<float>(i) + 0.5f);
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
    const float top = layout.height_px * layout.top_padding_frac;
    const float bot = layout.height_px * (1.f - layout.bottom_padding_frac);
    const float draw_h = bot - top;
    const double range = bounds.max - bounds.min;
    if (range <= 0.0) return (top + bot) * 0.5f;
    const double t = (price - bounds.min) / range;  // 0..1, 0 at min
    return bot - static_cast<float>(t) * draw_h;    // invert: high price → low y
}

}  // namespace vroom
