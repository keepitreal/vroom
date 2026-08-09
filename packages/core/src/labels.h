// Axis labels and their gridlines — both subsystems share a per-axis fade
// state so they animate in lockstep. When an interval swap happens (e.g.,
// 30m → 1h on x), the old labels and their gridlines fade out together, and
// the new ones fade in together.
//
// Free functions over `VroomChart&` rather than methods so the struct
// definition can stay in chart.h and the implementations can live here.

#pragma once

#include <cstdint>

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
struct PriceBounds;
}  // namespace vroom

namespace vroom::labels {

// Per-label fade state. Updated each frame: labels in the new active set get
// target=1 and fade in; labels falling out get target=0 and fade out.
struct YLabelFade {
    double price;
    float opacity = 0.f;
    float target = 1.f;
};

struct XLabelFade {
    int64_t time_ms;
    float opacity = 0.f;
    float target = 1.f;
};

// Opacity step per second (1 / 0.2s ≈ 200ms full fade). Paces the incremental
// per-label fades; an interval morph overrides them with the envelope below.
inline constexpr float kFadeRate = 5.f;

// How an axis behaves during an interval morph: the pre-switch ticks fade out
// over the first half, the new ones fade in over the second. Nothing translates
// — the tick set is swapped at the midpoint, while the axis is fully
// transparent, so a label is never seen moving between two positions.
//
// Both halves are clocked off the morph's own eased progress, so the axes take
// `transitionMs`, follow `transitionEasing`, and land with the candles.
struct IntervalPhase {
    bool active = false;    // false: fade per-label at kFadeRate as usual
    bool outgoing = false;  // true: first half — pre-switch ticks and scale
    float opacity = 1.f;    // whole-axis multiplier, 1 → 0 → 1
};

// Derived purely from the chart's morph state, so callers can ask for it freely.
IntervalPhase interval_phase(const VroomChart& chart);

// Y-axis (price) -----------------------------------------------------------

// Sets targets, walks the active price-interval set, advances opacities by
// `chart.last_dt_seconds`. Must be called once per frame before either
// `draw_y_gridlines` or `draw_y_labels`. `bounds` is the scale to lay ticks out
// against, which during an interval morph's outgoing half is the pre-switch one
// (`chart.morph_from_bounds`) — pass the same value to the draw calls.
void update_y_fades(VroomChart& chart,
                    const Layout& lay,
                    const PriceBounds& bounds);

void draw_y_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      const Layout& lay,
                      const PriceBounds& bounds,
                      float candle_right,
                      float candle_area_h);

void draw_y_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay,
                   const PriceBounds& bounds);

void gc_y_fades(VroomChart& chart);

// X-axis (time) ------------------------------------------------------------

// `start_ms`/`end_ms` are the window to lay ticks out against — the pre-switch
// one during an interval morph's outgoing half, `chart.visible_*` otherwise.
// Every x-axis call in a frame must be given the same window.
void update_x_fades(VroomChart& chart,
                    const Layout& lay,
                    int64_t start_ms,
                    int64_t end_ms);

void draw_x_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      int64_t start_ms,
                      int64_t end_ms,
                      float candle_area_w,
                      float candle_area_h);

void draw_x_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay,
                   int64_t start_ms,
                   int64_t end_ms);

void gc_x_fades(VroomChart& chart);

// Y-axis width sizing ------------------------------------------------------

// Recomputes `chart.axis_width_px` to fit the widest formatted price label
// at the current bounds. No-op if the typeface isn't loaded yet — the layout
// then falls back to `VROOM_FLOAT_Y_AXIS_WIDTH_RATIO`.
void recompute_axis_width(VroomChart& chart);

}  // namespace vroom::labels
