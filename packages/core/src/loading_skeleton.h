// The loading skeleton: a travelling wave of grey, semi-transparent candles
// standing in for a series that hasn't arrived yet.
//
// Drawn instead of the chart, not over it — draw_chart hands off here and
// returns, which is what suppresses the axis text, price badge, crosshair and
// indicator panes the real scene would otherwise paint.

#pragma once

#include <cstddef>

class SkCanvas;
struct VroomChart;

namespace vroom {
struct Layout;
}

namespace vroom::loading_skeleton {

// Bars the skeleton lays out across the plot at the chart's current candle
// width, clamped so a tiny or enormous canvas still gets a sane count.
std::size_t bar_count(const VroomChart& chart, const Layout& lay);

// Paints the whole skeleton frame: gridlines, waved candles, waved volume,
// the axis strip backgrounds, and the placeholder pills on both axes.
void draw(SkCanvas* canvas, const VroomChart& chart, const Layout& lay);

}  // namespace vroom::loading_skeleton
