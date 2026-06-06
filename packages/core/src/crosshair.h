// Crosshair drawing — two perpendicular dashed lines with a hollow ring at
// their intersection, kept in its own module alongside candles / labels so the
// chart orchestrator stays thin.
//
// Non-interactive for now: the intersection is parked at the center of the
// candle area. Once pointer tracking lands, the position will come from
// chart.crosshair_x_px / crosshair_y_px (see chart.h).

#pragma once

#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom::crosshair {

// Draws the crosshair within the candle area. `candle_right` is the x of the
// y-axis separator; `candle_area_h` is the y of the x-axis separator — the
// lines stop at those edges so they never bleed into the axis strips. `snap_x`
// is the (candle-snapped) x for the vertical line and ring; the horizontal line
// and the ring's y follow the touch y from `chart.crosshair_y_px`.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          float candle_right,
          float candle_area_h,
          float snap_x);

}  // namespace vroom::crosshair
