// Candle drawing — the wick+body loop, extracted so it can sit alongside
// future drawing modules (volume bars, indicator overlays, etc.) without
// growing the chart orchestrator.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

class SkCanvas;

namespace vroom {
struct Layout;
struct PriceBounds;
struct Theme;
}  // namespace vroom

namespace vroom::candles {

// Draws every candle in [visible, visible + n). The visible slice should
// already be filtered by `visible_indices` in viewport.h.
void draw(SkCanvas* canvas,
          const ::VroomCandle* visible,
          std::size_t n,
          const Layout& lay,
          const Theme& theme,
          const PriceBounds& bounds,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms);

}  // namespace vroom::candles
