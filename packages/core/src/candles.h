// Candle drawing — the wick+body loop, extracted so it can sit alongside
// future drawing modules (volume bars, indicator overlays, etc.) without
// growing the chart orchestrator.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

class SkCanvas;

namespace vroom {
struct CandleSnapshot;
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
//
// `from` / `from_n` is the outgoing geometry of an interval morph, indexed from
// the right of the visible slice (slot 0 = newest), and `morph_t` (0..1) is the
// eased progress toward `visible`. Each slot's wick and body interpolate between
// the two; slots present on only one side fade in or out. `morph_t == 1` (the
// default) draws `visible` alone.
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
          float opacity = 1.f,
          const CandleSnapshot* from = nullptr,
          std::size_t from_n = 0,
          float morph_t = 1.f);

}  // namespace vroom::candles
