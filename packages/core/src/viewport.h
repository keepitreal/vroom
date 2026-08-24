// Viewport math — pure functions, no Skia, no state.
// Translates between candle indices, time, price, and screen pixels.

#pragma once

#include <cstddef>
#include <cstdint>

#include "vroom/vroom_chart.h"

namespace vroom {

struct Layout {
    float width_px;             // total chart width
    float height_px;            // total chart height
    float y_axis_width_px;      // reserved on the right for price labels
    float x_axis_height_px;     // reserved at the bottom for time labels
    float right_padding_px;     // small gutter between candles and y-axis
    float candle_width_ratio;   // 0..1 of slot stride taken by the body
    float top_padding_frac;     // fraction of candle area; keeps prices off edges
    float bottom_padding_frac;
    float indicator_area_h;     // height reserved for below-chart indicator panes
};

// Bottom of the price (candle) pane = top of the indicator band. Shrinks when
// an indicator pane is present so candles, volume, and price labels reflow.
inline float price_pane_bottom(const Layout& l) {
    return l.height_px - l.x_axis_height_px - l.indicator_area_h;
}

// Top of the bottom time-axis strip. Stays anchored regardless of any indicator
// band (the strip is always the bottom x_axis_height_px of the chart).
inline float x_axis_top(const Layout& l) {
    return l.height_px - l.x_axis_height_px;
}

// Width available to candles: the full width minus the y-axis strip and the
// right-hand gutter.
inline float candle_area_width(const Layout& l) {
    return l.width_px - l.y_axis_width_px - l.right_padding_px;
}

struct PriceBounds {
    double min;
    double max;
};

// Half-open [start, end) range of candle indices.
struct IndexRange {
    size_t start;
    size_t end;
};

// One candle's geometry captured in normalized form for the interval morph:
// x as a fraction of the candle-area width, prices as fractions of the price
// band. Resolution-independent, so a resize mid-morph stays correct — and
// independent of the price bounds, so the captured geometry keeps its pixel
// position after a timeframe switch re-scales the y-axis.
struct CandleSnapshot {
    float x;
    float open, high, low, close;
    bool  bull;  // close >= open; selects the fill / wick / border color
};

// How many captured slots still contribute to a frame — 0 once the morph is
// done, which releases the capture. Drawing routines take max(n, this) as their
// slot count, pairing the new candle at slot k (visible[n - 1 - k]) with the
// captured one (from[k]), both counting back from the right edge.
inline std::size_t morph_from_count(const CandleSnapshot* from,
                                    std::size_t from_n,
                                    float morph_t) {
    return (from && morph_t < 1.f) ? from_n : 0;
}

// Returns the indices of candles whose time_ms falls in [start_ms, end_ms].
// When both are 0, returns the full range (Phase 1 default-everything behavior).
// Candles must be sorted ascending by time_ms (invariant of the public API).
IndexRange visible_indices(const ::VroomCandle* candles,
                           size_t count,
                           int64_t start_ms,
                           int64_t end_ms);

// Width of one candle body in pixels, computed from the time slot a single
// candle occupies (candle_duration_ms / window_ms × candle area).
float candle_body_width(const Layout& layout,
                        int64_t window_ms,
                        int64_t candle_duration_ms);

// Inverse of candle_body_width: the time window that makes each candle body
// `body_px` wide. Returns 0 if any input is degenerate. Does not consult the
// data span — a short series leaves empty space on the left (width wins).
int64_t window_for_body_width(const Layout& layout,
                              int64_t candle_duration_ms,
                              double body_px);

// A [start, end] time window of fixed length, used by pan/zoom clamping.
struct TimeWindow {
    int64_t start_ms;
    int64_t end_ms;
};

// Clamp a same-length time window after a pan or pinch-zoom shift.
//
// The future cap (`max_future`, typically 3/4 of the window) is unchanged: the
// right edge cannot run past last_time + max_future.
//
// The past cap depends on whether the window fits in the data:
// - Window ≤ data extent: start cannot precede first_time (no empty past).
// - Window > data extent: empty past is intentional (width wins). start MAY
//   precede first_time. Instead, the last candle cannot leave the right edge
//   (end >= last_time). Pinning start to first_time in this case would shove
//   the extra length into the future and latch, because the future cap then
//   fights the past cap on every subsequent pan.
TimeWindow clamp_shifted_time_window(int64_t start_ms, int64_t end_ms,
                                     int64_t first_time, int64_t last_time,
                                     int64_t candle_duration_ms,
                                     int64_t max_future);

// Center-x of a candle whose period starts at time_ms and lasts
// candle_duration_ms, given the current visible time window.
float candle_center_x(const Layout& layout,
                      int64_t time_ms,
                      int64_t candle_duration_ms,
                      int64_t visible_start_ms,
                      int64_t window_ms);

// Index of the candle whose center is nearest pixel x_px, searching
// [candles, candles+count). Clamps to the first/last candle. This is the
// integer counterpart of snap_x_to_candle — both share the same nearest-candle
// math. Precondition: count > 0 and window_ms > 0 (callers guard).
size_t snap_index_to_candle(const Layout& layout,
                            const ::VroomCandle* candles,
                            size_t count,
                            int64_t candle_duration_ms,
                            int64_t visible_start_ms,
                            int64_t window_ms,
                            float x_px);

// Result of snapping a pixel x to the candle grid. The grid extends past the
// last candle into the empty "future" space: slots there sit at
// last_candle.time_ms + k × candle_duration_ms and carry no candle data.
struct SnapResult {
    int64_t time_ms;     // period-start time of the snapped slot (real or future)
    bool has_candle;     // true when a real candle sits at this slot
    size_t index;        // index into [candles, candles+count); valid iff has_candle
};

// Snaps pixel x_px to the nearest candle-grid slot. Over the populated region
// this returns the nearest real candle (has_candle = true). To the right of the
// last candle it snaps to future grid slots (has_candle = false). x_px is
// clamped to the usable candle width so snapping never runs past the visible
// right edge. Precondition: count > 0 and window_ms > 0 (callers guard).
SnapResult snap_to_slot(const Layout& layout,
                        const ::VroomCandle* candles,
                        size_t count,
                        int64_t candle_duration_ms,
                        int64_t visible_start_ms,
                        int64_t window_ms,
                        float x_px);

// On-screen x of the slot whose center is nearest pixel x_px, searching
// [candles, candles+count) and the future grid past the last candle. Returns
// x_px unchanged when there are no candles or the window is degenerate. Used to
// snap the crosshair to candles (and to future candle-aligned slots).
float snap_x_to_candle(const Layout& layout,
                       const ::VroomCandle* candles,
                       size_t count,
                       int64_t candle_duration_ms,
                       int64_t visible_start_ms,
                       int64_t window_ms,
                       float x_px);

// Free (non-snapped, non-candle-centered) mapping between time and pixel x,
// used for drawing-tool endpoints that can sit anywhere — not just on a candle
// slot. `x_at_time` is the exact inverse of `time_at_x`:
//   frac = (time - start) / window;  x = usable * frac
// where `usable` is the candle area width (width - y-axis - right padding).
// Returns 0 / visible_start_ms respectively for a degenerate window/usable.
int64_t time_at_x(const Layout& layout,
                  int64_t visible_start_ms,
                  int64_t window_ms,
                  float x_px);
float x_at_time(const Layout& layout,
                int64_t visible_start_ms,
                int64_t window_ms,
                int64_t time_ms);

// Min/max of (low..high) across the given range.
PriceBounds price_bounds(const ::VroomCandle* candles, size_t count);

// Widening applied to auto-fit price bounds so candles keep some headroom
// instead of touching the pane edges. 1.0 = snug; larger = wider.
constexpr double kAutoYZoom = 1.5;

// price_bounds() widened about its midpoint by kAutoYZoom. This is the y-range
// used whenever the price scale is in auto (follow-the-data) mode. Returns the
// {0, 1} sentinel when count == 0 (callers keep their previous bounds).
PriceBounds auto_price_bounds(const ::VroomCandle* candles, size_t count);

// Rescales an axis range so a data envelope keeps the pixel height *and* the
// pixel position it had before a data swap — the "scale lock" applied on a
// timeframe switch, where the same price action re-buckets into a smaller or
// larger high-low span.
//
// `old_axis` is the axis range that was in effect; `old_env` / `new_env` are the
// visible high-low envelopes (as returned by price_bounds) before and after the
// swap. Resolution-independent: both envelopes share the same draw band, so the
// band height cancels out. Returns `old_axis` unchanged when any span is
// degenerate.
PriceBounds preserve_envelope_bounds(const PriceBounds& old_axis,
                                     const PriceBounds& old_env,
                                     const PriceBounds& new_env);

// Map a price to y in pixels. y=0 is top of the chart.
float price_to_y(const Layout& layout, const PriceBounds& bounds, double price);

// Fraction of the price band a price sits at: 0 = bounds.min, 1 = bounds.max.
// Returns 0.5 for a degenerate range, matching price_to_y's midpoint fallback.
double price_fraction(const PriceBounds& bounds, double price);

// The y of a band fraction. Splits price_to_y in two so geometry can be stored
// independently of the price bounds (see CandleSnapshot):
//   price_to_y(l, b, p) == y_at_fraction(l, price_fraction(b, p))
float y_at_fraction(const Layout& layout, double frac);

// Inverse of price_to_y: map a pixel y in the price pane back to a price.
// Returns bounds.min for a degenerate range or draw band.
double y_to_price(const Layout& layout, const PriceBounds& bounds, float y);

}  // namespace vroom
