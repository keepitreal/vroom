// Tick algorithms — pick a label interval based on current scale so labels
// land on canonical boundaries with reasonable spacing.
//
// Pure functions, no state. Used by the label subsystem; safe for future
// indicators that need their own axis grids to reuse.

#pragma once

#include <cstddef>
#include <cstdint>

namespace vroom {

// Minimum spacing between adjacent x-axis labels in pixels. Smaller → more
// labels visible at every zoom level; larger → sparser.
inline constexpr float kXLabelMinSpacing = 50.f;

// Target spacing for y-axis labels — used to derive a target count.
inline constexpr float kYLabelTargetSpacing = 55.f;

// X-axis tick cadence. The hierarchy starts with fixed millisecond intervals
// (1m … 1w) and, beyond a week, switches to calendar-aware month/year steps so
// labels land on real month and year boundaries instead of drifting epoch
// multiples.
enum class TimeUnit { Fixed, Month, Year };

// A chosen tick cadence. For Fixed, ticks land on epoch multiples of step_ms.
// For Month/Year, ticks land on local-calendar boundaries `step` units apart
// (e.g. {Month, step=3} → quarter starts; {Year, step=1} → Jan 1 each year).
struct TimeTick {
    TimeUnit unit;
    int64_t  step_ms;  // valid iff unit == Fixed
    int      step;     // valid iff unit == Month or Year
};

// Selects the coarsest-enough cadence whose on-screen spacing meets the minimum.
// Hierarchy: 1m → 5m → 15m → 30m → 1h → 2h → 4h → 6h → 12h → 1d → 2d → 1w →
// 1mo → 3mo → 6mo → 1y → 2y → 5y → … (the year step grows as 1/2/5 × 10ⁿ so
// arbitrarily wide windows never overlap).
TimeTick pick_time_tick(int64_t window_ms, float candle_area_w);

// First tick at or after from_ms for the given cadence. Calendar units align to
// local month/year boundaries.
int64_t first_tick_at_or_after(int64_t from_ms, const TimeTick& tick);

// The next tick strictly after t for the given cadence.
int64_t next_tick(int64_t t, const TimeTick& tick);

// "Nice number" tick selection for the y-axis: snaps to 1, 2, or 5 × 10ⁿ
// based on the price range and target spacing. Returns the price interval
// in the same units as the data (e.g. dollars).
double pick_price_interval(double range, float candle_area_h);

// Decimal places needed so adjacent ticks `interval` apart don't collapse to
// the same string. 0.01 → 2, 0.005 → 3, 1e-8 → 8. Clamped to [0, 12].
int price_decimals(double interval);

// Writes `price` into `buf` using `decimals` (from price_decimals).
void format_price(char* buf, size_t buf_size, double price, int decimals);

}  // namespace vroom
