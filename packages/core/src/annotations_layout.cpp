#include "annotations_layout.h"

#include <algorithm>

#include "vroom/vroom_chart.h"

namespace vroom::annotations {

float marker_size(float size_px) {
    return size_px > 0.f ? size_px : kDefaultMarkerSize;
}

std::array<Pt, 3> triangle(int32_t shape, float cx, float cy, float size) {
    const float h = size * 0.5f;
    if (shape == VROOM_ANNOTATION_TRIANGLE_UP) {
        return {{{cx, cy - h}, {cx + h, cy + h}, {cx - h, cy + h}}};
    }
    return {{{cx, cy + h}, {cx - h, cy - h}, {cx + h, cy - h}}};
}

float label_left(int32_t align, float anchor_x, float text_w) {
    if (align == 1) return anchor_x - text_w * 0.5f;
    if (align == 2) return anchor_x - text_w;
    return anchor_x;
}

bool overlaps_plot(float left, float right, float clip_right) {
    return std::max(left, right) >= 0.f && std::min(left, right) <= clip_right;
}

}  // namespace vroom::annotations
