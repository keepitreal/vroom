// Current-price indicator — a dotted horizontal line at the latest candle's
// close, spanning the candle area and ending in a rounded price tag in the
// y-axis strip. Red/green by whether the latest period closed up or down. Kept
// in its own module alongside candles / labels / crosshair so the chart
// orchestrator stays thin.

#pragma once

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::price_indicator {

// Draws the line across [0, candle_right] at the latest close's y, plus the
// price box in the y-axis strip. `candle_area_h` is the y of the x-axis
// separator; the indicator is skipped if the close maps outside [0, candle_area_h].
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          float candle_right,
          float candle_area_h);

}  // namespace vroom::price_indicator
