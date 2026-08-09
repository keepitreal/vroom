// Moving-average overlay line — a single SMA/EMA polyline drawn on the price
// pane (over the candles, on the candle price scale). One call per overlay line.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"  // ::VroomCandle

class SkCanvas;

namespace vroom {
struct CandleSnapshot;
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::ma_overlay {

// Draws `values_visible` (aligned with `visible`, NaN where undefined) as a
// polyline on the price pane using candle_center_x + price_to_y. `color` is
// 0xAARRGGBB; `width` is the stroke px. Clipped to the candle area.
//
// `break_before` (optional, aligned with `visible`) forces a new subpath at any
// index where it is non-zero — used by VWAP to break the line at session
// resets. Pass nullptr for continuous lines (SMA/EMA).
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
          float width,
          const unsigned char* break_before = nullptr,
          float opacity = 1.f);

// The close-price polyline of line-chart mode. Equivalent to draw() fed the
// visible closes, plus the interval morph: `from` / `from_n` is the outgoing
// geometry indexed from the right of the visible slice (slot 0 = newest) and
// `morph_t` (0..1) is the eased progress, so each vertex slides from the close it
// had before the timeframe switch to its new one. `morph_t == 1` (the default)
// draws `visible` alone.
//
// Separate from draw() because the capture holds candle closes — it can't stand
// in for an indicator's values.
void draw_close_line(SkCanvas* canvas,
                     const Layout& lay,
                     const PriceBounds& bounds,
                     const ::VroomCandle* visible,
                     std::size_t n,
                     int64_t window_ms,
                     int64_t visible_start_ms,
                     int64_t candle_duration_ms,
                     float candle_right,
                     float candle_area_h,
                     uint32_t color,
                     float width,
                     float opacity = 1.f,
                     const CandleSnapshot* from = nullptr,
                     std::size_t from_n = 0,
                     float morph_t = 1.f);

// Fills the closed region between two aligned series (NaN where undefined)
// with `color` at its alpha × `opacity` — used for the Bollinger Band fill.
// Runs where either series is NaN are skipped, so the fill never bridges the
// warmup gap. Plain-alpha SkPaint fill, no gradient shader (the Skia gradient
// APIs diverge across our pinned versions; see liquidity.cpp). Clipped to the
// candle area like draw().
void fill_between(SkCanvas* canvas,
                  const Layout& lay,
                  const PriceBounds& bounds,
                  const ::VroomCandle* visible,
                  std::size_t n,
                  const double* upper_visible,
                  const double* lower_visible,
                  int64_t window_ms,
                  int64_t visible_start_ms,
                  int64_t candle_duration_ms,
                  float candle_right,
                  float candle_area_h,
                  uint32_t color,
                  float opacity);

}  // namespace vroom::ma_overlay
