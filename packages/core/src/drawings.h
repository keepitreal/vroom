// Drawing annotations — user-placed line tools rendered on the price pane.
//
// Two kinds of geometry are drawn here:
//   * Committed shapes (chart.drawings) — lines, boxes, freehand strokes and
//     arrow-tipped paths, anchored in data space so they stay glued to the
//     candles as the user pans/zooms.
//   * The transient "draft" (chart.draft_*) — the in-progress node dots and the
//     live guideline shown while the user is placing a shape.
//
// Like ma_overlay / crosshair this is its own module so the chart orchestrator
// stays thin. Drawn after the overlays and before the axis backgrounds.

#pragma once

#include "viewport.h"
#include "vroom/vroom_chart.h"

class SkCanvas;
struct VroomChart;

namespace vroom::drawings {

// Draws the committed line drawings and the in-progress draft. `candle_right` is
// the x of the y-axis strip and `candle_area_h` the price-pane bottom; geometry
// is clipped to that rectangle so lines never bleed into the axis strips or the
// indicator panes below. Node dots are exempt from the clip so a dot placed at
// the very edge still renders fully.
void draw(SkCanvas* canvas,
          const VroomChart& chart,
          const vroom::Layout& lay,
          const vroom::PriceBounds& bounds,
          float candle_right,
          float candle_area_h);

// Result of a hit-test against the committed drawings. `index` is the drawing
// index (or -1 for a miss). `part` depends on the drawing kind:
//   * line   — 0 (endpoint A), 1 (endpoint B), or 2 (line body).
//   * box    — 0..3 (the four corners, in the order returned by box_corners:
//              a, (b.x,a.y), b, (a.x,b.y)) or 4 (box body: interior or edges).
//   * pencil — always 5 (stroke body). A freehand stroke has no grab handles:
//              its end anchors are a visual cue only and translate the whole
//              path like any other part of it.
//   * path   — 6 (path body) or VROOM_DRAW_PART_VERTEX + i for vertex i. Unlike
//              a pencil every vertex was placed deliberately, so every one is a
//              grab handle; the offset keeps those indices clear of the small
//              part numbers above.
// Corner/endpoint/vertex hits are only reported for the currently selected
// drawing (whose handles are visible).
struct HitResult {
    int32_t index;
    int32_t part;
    float   t;  // 0..1 grab position along a line segment (A→B); 0 for box/handle hits
};

HitResult hit_test(const VroomChart& chart,
                   const vroom::Layout& lay,
                   const vroom::PriceBounds& bounds,
                   int64_t window_ms,
                   float x,
                   float y);

}  // namespace vroom::drawings
