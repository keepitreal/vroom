// MACD indicator pane — histogram bars, MACD + signal lines, and a zero line,
// drawn in a band below the candles. Its own module alongside rsi_pane so the
// chart orchestrator stays thin.

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

namespace vroom::macd_pane {

// Draws the MACD pane spanning [pane_top, pane_bottom]. The macd/signal/hist
// series are aligned with `visible` (NaN where undefined). The vertical scale
// auto-fits the visible values symmetrically about zero; shares the candles'
// horizontal mapping (candle_center_x) so it scrolls in lock-step.
//
// The three `*_from` captures plus `morph_t` are the interval morph: each holds
// its series' outgoing shape in fractions of the band, indexed from the right
// (slot 0 = newest), so the pane can re-fit across the switch without the shape
// moving. Histogram bars reshape the way candle bodies do — their tops slide
// while the zero line, which is fixed to the band center, stays put. A fade's
// outgoing half passes n = 0 with morph_t = 0, painting the captures alone.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const Layout& lay,
          const ::VroomCandle* visible,
          std::size_t n,
          const double* macd_visible,
          const double* signal_visible,
          const double* hist_visible,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float candle_right,
          float pane_top,
          float pane_bottom,
          const LineMorph* macd_from = nullptr,
          const LineMorph* signal_from = nullptr,
          const LineMorph* hist_from = nullptr,
          float morph_t = 1.f);

}  // namespace vroom::macd_pane
