// Viewport math — pure functions, no Skia, no state.
// Translates between candle indices, time, price, and screen pixels.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

namespace vroom {

struct Layout {
    float width_px;             // total chart width
    float height_px;            // total chart height
    float y_axis_width_px;      // reserved on the right for price labels
    float x_axis_height_px;     // reserved at the bottom for time labels
    float right_padding_px;     // small gutter between candles and y-axis
    float candle_width_ratio;   // 0..1 of slot stride taken by the body
    float top_padding_frac;     // fraction of candle area; keeps prices off edges
    float bottom_padding_frac;
};

struct PriceBounds {
    double min;
    double max;
};

// Half-open [start, end) range of candle indices.
struct IndexRange {
    size_t start;
    size_t end;
};

// Returns the indices of candles whose time_ms falls in [start_ms, end_ms].
// When both are 0, returns the full range (Phase 1 default-everything behavior).
// Candles must be sorted ascending by time_ms (invariant of the public API).
IndexRange visible_indices(const ::VroomCandle* candles,
                           size_t count,
                           int64_t start_ms,
                           int64_t end_ms);

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
