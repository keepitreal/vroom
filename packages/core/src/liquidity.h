// Liquidity bands — resting-order / order-book depth rendered on the price pane.
//
// Each band (chart.bands) is a price interval carrying a total size on one side
// of the book. It is drawn as a horizontal rectangle anchored at the inner edge
// of the price axis (`candle_right`), stretching left by the configured reach
// and fading to transparent with a left->right gradient. Color comes from the
// band's side (buy/sell); the band's alpha comes from its volume mapped into
// [min_opacity, max_opacity]. Because the vertical extent is derived from price
// via price_to_y, bands scale with the y-axis on zoom.
//
// Like ma_overlay / drawings this is its own module so the chart orchestrator
// stays thin. Drawn BEHIND the candles (before candles::draw) so candle bodies
// paint over the bands.

#pragma once

#include "viewport.h"
#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom::liquidity {

// Draws the liquidity bands. `candle_right` is the x of the price-axis strip
// (bands' right edge); `candle_area_h` is the price-pane bottom. Geometry is
// clipped to that rectangle so bands never bleed into the axis strips.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const vroom::Layout& lay,
          const vroom::PriceBounds& bounds,
          float candle_right,
          float candle_area_h);

}  // namespace vroom::liquidity
