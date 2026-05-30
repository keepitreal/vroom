// Viewport math — pure functions, no Skia, no state.
// Translates between candle indices, time, price, and screen pixels.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

namespace vroom {

struct Layout {
    float width_px;
    float height_px;
    float right_padding_px;     // gutter on the right edge so newest candle isn't flush
    float candle_width_ratio;   // 0..1 of slot stride taken by the body
    float top_padding_frac;     // fraction of height; keeps prices off the edges
    float bottom_padding_frac;
};

struct PriceBounds {
    double min;
    double max;
};

// Width of one candle body in pixels, given count and layout.
// Stride (body + gap) is derived from candle_width_ratio; the gap is the
// remainder of the slot.
float candle_body_width(const Layout& layout, size_t count);

// Center-x of candle `i` in pixels.
float candle_center_x(const Layout& layout, size_t count, size_t i);

// Min/max of (low..high) across the given range.
PriceBounds price_bounds(const ::VroomCandle* candles, size_t count);

// Map a price to y in pixels. y=0 is top of the chart.
float price_to_y(const Layout& layout, const PriceBounds& bounds, double price);

}  // namespace vroom
