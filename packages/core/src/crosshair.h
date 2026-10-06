// Crosshair drawing — two perpendicular dashed lines with a hollow ring at
// their intersection, kept in its own module alongside candles / labels so the
// chart orchestrator stays thin.
//
// Non-interactive for now: the intersection is parked at the center of the
// candle area. Once pointer tracking lands, the position will come from
// chart.crosshair_x_px / crosshair_y_px (see chart.h).

#pragma once

#include <cstdint>

#include "crosshair_button.h"
#include "viewport.h"
#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom::crosshair {

// Draws the crosshair. `candle_right` is the x of the y-axis strip. The
// horizontal line and ring live in the price pane: the ring's y is clamped to
// `candle_area_h` (the price-pane bottom). The horizontal line runs to the
// price badge. The vertical line's x stays inside the plot and spans from the
// top down to `vline_bottom` (the bottom of the indicator region, so it stays
// visible over any RSI/MACD panes); pass `candle_area_h` for both when there
// are no panes. `snap_x` is the candle-snapped x for the vertical line and ring.
//
// Two axis badges are drawn over the strips: a date/time badge (formatted from
// `snap_time_ms`) centered on the vertical line in the x-axis strip, and a
// price badge (the price at the crosshair y, via `lay`/`bounds`) centered in
// the y-axis strip. Both render on top of the axis labels and the current-price
// indicator since the crosshair is the last thing drawn.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const vroom::Layout& lay,
          const vroom::PriceBounds& bounds,
          float candle_right,
          float candle_area_h,
          float vline_bottom,
          float snap_x,
          int64_t snap_time_ms);

// The horizontal line's y: the pinned price's y while the plus button is
// pinned, the pointer's otherwise; clamped into the price pane.
float line_y(const VroomChart& chart,
             const vroom::Layout& lay,
             const vroom::PriceBounds& bounds,
             float candle_area_h);

// Where the plus button draws, from the same measurements as `draw`. False when
// the button is disabled or the crosshair is down. `price` (optional) gets the
// price at the line.
bool button_rect(const VroomChart& chart,
                 const vroom::Layout& lay,
                 const vroom::PriceBounds& bounds,
                 float candle_right,
                 float candle_area_h,
                 crosshair_button::Rect* out,
                 double* price);

}  // namespace vroom::crosshair
