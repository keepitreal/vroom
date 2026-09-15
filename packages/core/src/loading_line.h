// The loading line: a single stroke across the plot standing in for a series
// that hasn't arrived yet, and the bridge the real candles grow out of.
//
// Three stages, all drawn by the one function here:
//
//   1. idle    — a travelling sine across the full width (see loading_wave.h)
//   2. morph   — that sine, frozen, easing into a polyline through the vertical
//                centre of every candle about to be drawn
//   3. reveal  — the polyline holding still at those centres while it fades
//                out and the candles grow outward from it
//
// Stages 1 and 2 run instead of the chart, so the axis text, price badge,
// crosshair and indicator panes stay suppressed until the data is really
// there. Stage 3 runs over the top of the normal scene.

#pragma once

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
}

namespace vroom::loading_line {

// Stages 1 and 2, in place of the chart. Which one it draws depends on whether
// the chart holds a capture (see VroomChart::begin_loading_morph).
void draw(SkCanvas* canvas, const VroomChart& chart, const Layout& lay);

// Stage 3, over the settled scene. `alpha` is the line's remaining opacity, so
// the caller can ride it on the same clock as the candles' growth.
void draw_fading(SkCanvas* canvas,
                 const VroomChart& chart,
                 const Layout& lay,
                 float alpha);

}  // namespace vroom::loading_line
