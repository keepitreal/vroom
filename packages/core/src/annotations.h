// Host-supplied annotations (vroom_chart_set_annotations): read-only boxes,
// segments, markers and labels anchored in data space, drawn over the candles
// and overlay lines: boxes first, so a translucent fill highlights the bars it
// spans, then everything else on top of them.

#pragma once

#include <cstdint>

#include "viewport.h"

class SkCanvas;
struct VroomChart;

namespace vroom::annotations {

// Box fills and borders, clipped to the price pane.
void draw_boxes(SkCanvas* canvas,
                const VroomChart& chart,
                const vroom::Layout& lay,
                const vroom::PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h);

// Segments, then markers, then labels, clipped to the price pane. Labels are
// skipped while no typeface is loaded.
void draw_marks(SkCanvas* canvas,
                const VroomChart& chart,
                const vroom::Layout& lay,
                const vroom::PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h);

}  // namespace vroom::annotations
