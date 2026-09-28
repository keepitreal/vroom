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

// Gap between the widest y-axis badge and each edge of the strip. Equal on
// both sides so the shared column (`width - axis_width / 2`) leaves this much
// inside the canvas on the right, where the viewport clips.
inline constexpr float kAxisInset = 8.f;

// Horizontal padding inside a price badge (current price, crosshair, and the
// price-line axis pill). Matches `price_lines::kPadH`; the strip is sized to
// the badge, not the bare text, so this has to stay in step with it.
inline constexpr float kAxisBadgePadH = 8.f;

// Widest badge that shares the column. `axis_text_w` is the widest price
// string at the axis font; `price_line_text_w` is the widest at the price-line
// font. Each side adds the badge pad. Bare tick text is the string alone, so
// it is always narrower than the axis-font badge of the same string.
inline float axis_content_width(float axis_text_w, float price_line_text_w) {
    const float pad = 2.f * kAxisBadgePadH;
    const float axis_badge = axis_text_w + pad;
    const float line_badge = price_line_text_w + pad;
    return axis_badge > line_badge ? axis_badge : line_badge;
}

// Strip that holds `content_w` with `kAxisInset` on both sides.
inline float axis_strip_width(float content_w) {
    return content_w + 2.f * kAxisInset;
}

// Left edge of a price badge centered on the y-axis column. `text_w` is the
// formatted price's advance at the font that badge draws with. A stroke that
// belongs to the badge ends here so it meets the pill; the plot still stops
// at the candle area's right edge.
inline float axis_badge_left(float width_px, float y_axis_width_px, float text_w) {
    const float cx = width_px - y_axis_width_px * 0.5f;
    const float box_w = text_w + 2.f * kAxisBadgePadH;
    return cx - box_w * 0.5f;
}

// Recomputes `chart.axis_width_px` to fit the widest price badge at the scale
// the labels will draw against (visible auto-fit, or the manual scale). No-op
// width (0) if the typeface isn't loaded yet — the layout then falls back to
// `VROOM_FLOAT_Y_AXIS_WIDTH_RATIO`.
void recompute_axis_width(VroomChart& chart);

}  // namespace vroom::labels
