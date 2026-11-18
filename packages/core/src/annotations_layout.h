// Annotation geometry — pure functions, no Skia, no state, so the shapes the
// draw pass emits unit-test without a Skia checkout.

#pragma once

#include <array>
#include <cstdint>

namespace vroom::annotations {

constexpr float kDefaultMarkerSize = 10.f;
constexpr float kLabelPadX = 4.f;
constexpr float kLabelPadY = 2.f;

struct Pt {
    float x;
    float y;
};

// The size a marker draws at: `size_px`, or the default when unset.
float marker_size(float size_px);

// Triangle vertices centered on (cx, cy), `size` wide and tall. TRIANGLE_UP
// points up (apex first); anything else is treated as TRIANGLE_DOWN.
std::array<Pt, 3> triangle(int32_t shape, float cx, float cy, float size);

// Left edge of a `text_w`-wide label anchored at `anchor_x` under `align`
// (0 = starts there, 1 = centered, 2 = ends there).
float label_left(int32_t align, float anchor_x, float text_w);

// Whether [left, right] reaches into the plot [0, clip_right] at all.
bool overlaps_plot(float left, float right, float clip_right);

}  // namespace vroom::annotations
