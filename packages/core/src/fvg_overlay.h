// Fair Value Gap boxes — the drawing half of the indicator, kept out of the
// chart orchestrator the way liquidity / drawings are. Detection lives in the
// Skia-free fair_value_gaps.{h,cpp}; this reads the cache it fills.
//
// Each gap becomes an axis-aligned rectangle spanning its untouched price range
// and running right from the middle bar. Split across two passes because they
// sit at different depths: the boxes go BEHIND the candles so bodies paint over
// them, the labels go in FRONT so they stay readable.

#pragma once

#include <cstdint>

#include "viewport.h"

class SkCanvas;
struct VroomChart;

namespace vroom::fvg_overlay {

// Draws the box fills and their outlines. `candle_right` is the x of the
// price-axis strip and `candle_area_h` the price-pane bottom; geometry is
// clipped to that rectangle so a box never bleeds into the axis strips.
void draw_boxes(SkCanvas* canvas,
                const VroomChart& chart,
                const vroom::Layout& lay,
                const vroom::PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h);

// Draws each box's label: inside its right end normally, or out in the empty
// slots past the newest candle when the boxes are extended. No-op while no
// typeface is loaded.
void draw_labels(SkCanvas* canvas,
                 const VroomChart& chart,
                 const vroom::Layout& lay,
                 const vroom::PriceBounds& bounds,
                 int64_t window_ms,
                 float candle_right,
                 float candle_area_h);

}  // namespace vroom::fvg_overlay
