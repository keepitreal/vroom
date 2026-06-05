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

// Opacity step per second (1 / 0.2s ≈ 200ms full fade).
inline constexpr float kFadeRate = 5.f;

// Y-axis (price) -----------------------------------------------------------

// Sets targets, walks the active price-interval set, advances opacities by
// `chart.last_dt_seconds`. Must be called once per frame before either
// `draw_y_gridlines` or `draw_y_labels`.
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

void update_x_fades(VroomChart& chart, const Layout& lay);

void draw_x_gridlines(SkCanvas* canvas,
                      const VroomChart& chart,
                      float candle_area_w,
                      float candle_area_h);

void draw_x_labels(SkCanvas* canvas,
                   const VroomChart& chart,
                   const Layout& lay);

void gc_x_fades(VroomChart& chart);

// Y-axis width sizing ------------------------------------------------------

// Recomputes `chart.axis_width_px` to fit the widest formatted price label
// at the current bounds. No-op if the typeface isn't loaded yet — the layout
// then falls back to `VROOM_FLOAT_Y_AXIS_WIDTH_RATIO`.
void recompute_axis_width(VroomChart& chart);

}  // namespace vroom::labels
