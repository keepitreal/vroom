// RSI indicator pane — draws the RSI line, 70/30 reference lines, axis labels,
// and a caption in the band below the candles. Its own module alongside
// candles / volume / price_indicator so the chart orchestrator stays thin.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"  // ::VroomCandle

class SkCanvas;

namespace vroom {
struct Layout;
struct LineMorph;
}  // namespace vroom

struct VroomChart;

namespace vroom::rsi_pane {

// Draws the RSI pane spanning vertically [pane_top, pane_bottom]. `rsi_visible`
// is the cached RSI series aligned with `visible` (NaN where undefined). Shares
// the candles' horizontal mapping (candle_center_x) so it scrolls in lock-step.
//
// `rsi_from` / `rsi_ma_from` / `morph_t` are the interval morph: each capture
// holds its line's outgoing shape indexed from the right (slot 0 = newest) and
// every vertex slides from where it sat before the timeframe switch to where it
// sits now. A fade's outgoing half passes n = 0 with morph_t = 0, which paints
// the captures alone.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* rsi_visible,
          const double* rsi_ma_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom,
          const LineMorph* rsi_from = nullptr,
          const LineMorph* rsi_ma_from = nullptr,
          float morph_t = 1.f);

}  // namespace vroom::rsi_pane
