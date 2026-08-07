// Consumer-supplied horizontal price status lines — the primitive behind resting
// limit orders, take-profits and liquidation levels.
//
// Each line is a styled horizontal rule across the price pane ending in a label
// group (body pill + optional quantity pill + optional close button) and an
// optional price badge in the y-axis strip, closely modeled on the current-price
// indicator it sits beside. Lines can be dragged vertically and closed; the
// interaction state (hover, live drag price) lives on the chart and is driven by
// the host's gesture layer through the public facade.
//
// Geometry is computed in price_line_layout.h so the draw and hit-test passes
// agree by construction.

#pragma once

#include <cstdint>

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::price_lines {

// Draws every price line. `candle_right` is the inner edge of the y-axis strip
// and `candle_area_h` the y of the x-axis separator; a line whose price maps
// outside [0, candle_area_h] is skipped rather than clamped.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h);

// A hit against a price line. `index` is -1 for a miss; `part` is 0 for the line
// or its label body (the drag target) and 1 for the close button.
struct HitResult {
    int32_t index;
    int32_t part;
};

// Hit-tests pixel (x, y). Close buttons win over lines, and among several
// candidates the nearest in y wins. Only draggable lines report part 0 and only
// closable lines report part 1.
HitResult hit_test(const VroomChart& chart,
                   const Layout& lay,
                   const PriceBounds& bounds,
                   float candle_right,
                   float candle_area_h,
                   float x,
                   float y);

}  // namespace vroom::price_lines
