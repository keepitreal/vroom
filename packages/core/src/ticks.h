// Tick algorithms — pick a label interval based on current scale so labels
// land on canonical boundaries with reasonable spacing.
//
// Pure functions, no state. Used by the label subsystem; safe for future
// indicators that need their own axis grids to reuse.

#pragma once

#include <cstdint>

namespace vroom {

// Minimum spacing between adjacent x-axis labels in pixels. Smaller → more
// labels visible at every zoom level; larger → sparser.
inline constexpr float kXLabelMinSpacing = 50.f;

// Target spacing for y-axis labels — used to derive a target count.
inline constexpr float kYLabelTargetSpacing = 55.f;

// Selects the smallest time interval whose pixel-spacing meets the minimum.
// Returns milliseconds. Hierarchy: 1m → 5m → 15m → 30m → 1h → 2h → 4h → 6h →
// 12h → 1d → 2d → 1w.
int64_t pick_time_interval(int64_t window_ms, float candle_area_w);

// "Nice number" tick selection for the y-axis: snaps to 1, 2, or 5 × 10ⁿ
// based on the price range and target spacing. Returns the price interval
// in the same units as the data (e.g. dollars).
double pick_price_interval(double range, float candle_area_h);

}  // namespace vroom
