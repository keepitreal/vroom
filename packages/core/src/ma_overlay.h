// Moving-average overlay line — a single SMA/EMA polyline drawn on the price
// pane (over the candles, on the candle price scale). One call per overlay line.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"  // ::VroomCandle

class SkCanvas;

namespace vroom {
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::ma_overlay {

// Draws `values_visible` (aligned with `visible`, NaN where undefined) as a
// polyline on the price pane using candle_center_x + price_to_y. `color` is
// 0xAARRGGBB; `width` is the stroke px. Clipped to the candle area.
void draw(SkCanvas* canvas,
          const Layout& lay,
          const PriceBounds& bounds,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* values_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float candle_area_h,
          uint32_t color,
          float width);

}  // namespace vroom::ma_overlay
