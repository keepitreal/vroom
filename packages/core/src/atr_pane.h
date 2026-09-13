// ATR indicator pane — draws the Average True Range line, its caption, and a
// peak label in the band below the candles. Its own module alongside
// rsi_pane / macd_pane so the chart orchestrator stays thin.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"  // ::VroomCandle

class SkCanvas;

namespace vroom {
struct Layout;
}  // namespace vroom

struct VroomChart;

namespace vroom::atr_pane {

// Draws the ATR pane spanning vertically [pane_top, pane_bottom]. `atr_visible`
// is the cached ATR series aligned with `visible` (NaN where undefined). ATR is
// strictly positive and unbounded, so the pane fits 0..peak anchored at its
// bottom edge rather than centering on a reference level.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* atr_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom);

}  // namespace vroom::atr_pane
