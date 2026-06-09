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

// Width of one candle body in pixels, computed from the time slot a single
// candle occupies (candle_duration_ms / window_ms × candle area).
float candle_body_width(const Layout& layout,
                        int64_t window_ms,
                        int64_t candle_duration_ms);

// Center-x of a candle whose period starts at time_ms and lasts
// candle_duration_ms, given the current visible time window.
float candle_center_x(const Layout& layout,
                      int64_t time_ms,
                      int64_t candle_duration_ms,
                      int64_t visible_start_ms,
                      int64_t window_ms);

// Index of the candle whose center is nearest pixel x_px, searching
// [candles, candles+count). Clamps to the first/last candle. This is the
// integer counterpart of snap_x_to_candle — both share the same nearest-candle
// math. Precondition: count > 0 and window_ms > 0 (callers guard).
size_t snap_index_to_candle(const Layout& layout,
                            const ::VroomCandle* candles,
                            size_t count,
                            int64_t candle_duration_ms,
                            int64_t visible_start_ms,
                            int64_t window_ms,
                            float x_px);

// On-screen x of the candle whose center is nearest pixel x_px, searching
// [candles, candles+count). Clamps to the first/last candle. Returns x_px
// unchanged when there are no candles or the window is degenerate. Used to snap
// the crosshair to candles.
float snap_x_to_candle(const Layout& layout,
                       const ::VroomCandle* candles,
                       size_t count,
                       int64_t candle_duration_ms,
                       int64_t visible_start_ms,
                       int64_t window_ms,
                       float x_px);

// Min/max of (low..high) across the given range.
PriceBounds price_bounds(const ::VroomCandle* candles, size_t count);

// Map a price to y in pixels. y=0 is top of the chart.
float price_to_y(const Layout& layout, const PriceBounds& bounds, double price);

}  // namespace vroom
