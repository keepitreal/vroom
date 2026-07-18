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
//
// `collapse` (0..1) morphs each candle vertically toward its close price for the
// candle→line transition: 0 = normal candle, 1 = a flat point at the close (the
// body/wick heights and width shrink to the line). `opacity` (0..1) fades the
// whole candle layer out as the line fades in. Both default to a no-op.
void draw(SkCanvas* canvas,
          const ::VroomCandle* visible,
          std::size_t n,
          const Layout& lay,
          const Theme& theme,
          const PriceBounds& bounds,
          int64_t window_ms,
          int64_t visible_start_ms,
          int64_t candle_duration_ms,
          float collapse = 0.f,
          float opacity = 1.f);

}  // namespace vroom::candles
