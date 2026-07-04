// Drawing annotations — user-placed line tools rendered on the price pane.
//
// Two kinds of geometry are drawn here:
//   * Committed lines (chart.drawings) — two-point trendlines anchored in data
//     space, so they stay glued to the candles as the user pans/zooms.
//   * The transient "draft" (chart.draft_*) — the in-progress node dots and the
//     live guideline shown while the user is placing a line.
//
// Like ma_overlay / crosshair this is its own module so the chart orchestrator
// stays thin. Drawn after the overlays and before the axis backgrounds.

#pragma once

#include "viewport.h"
#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom::drawings {

// Draws the committed line drawings and the in-progress draft. `candle_right` is
// the x of the y-axis strip and `candle_area_h` the price-pane bottom; geometry
// is clipped to that rectangle so lines never bleed into the axis strips or the
// indicator panes below. Node dots are exempt from the clip so a dot placed at
// the very edge still renders fully.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const vroom::Layout& lay,
          const vroom::PriceBounds& bounds,
          float candle_right,
          float candle_area_h);

}  // namespace vroom::drawings
