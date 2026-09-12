// Footprints — circular badges marking where a trader entered and exited, drawn
// above the candle each trade fell in.
//
// A buy is a "+" in the bull color, a sell a "-" in the bear color, both glyphs in
// VROOM_COLOR_BADGE_TEXT. Only one badge per side per candle renders however many
// fills went into it; the host learns the full set through
// vroom_chart_footprints_at and draws its own tooltip.
//
// Bucketing and badge geometry live in footprints_layout.h so the draw and
// hit-test passes agree by construction.

#pragma once

#include <cstdint>

#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::footprints {

// Draws every visible footprint badge, clipped to the price pane. `window_ms` is
// the visible time span, used to place each badge over its candle's slot center.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const PriceBounds& bounds,
          int64_t window_ms);

// Hit-tests pixel (x, y) against the badges. The returned hit's `side` is -1 on a
// miss; when two badges overlap the nearest center wins. Derives geometry from the
// same layout functions as draw, so what the user sees is what they can hover.
::VroomFootprintHit hit_test(const VroomChart& chart,
                             const Layout& lay,
                             const PriceBounds& bounds,
                             float x,
                             float y);

}  // namespace vroom::footprints
