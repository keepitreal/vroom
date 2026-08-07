#include "price_line_layout.h"

#include <algorithm>
#include <cmath>

namespace vroom::price_lines {

namespace {
// A pill's width is its text plus symmetric padding. Zero-width text means the
// segment is absent, not a padding-only sliver.
float pill_width(float text_w) {
    return text_w > 0.f ? text_w + 2.f * kPadH : 0.f;
}
}  // namespace

bool contains(const Rect& r, float x, float y) {
    if (r.empty()) return false;
    return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

GroupLayout layout_group(const LabelMetrics& metrics,
                         float y,
                         float pane_right,
                         const ::VroomPriceLineStyle& style) {
    GroupLayout out;

    const float body_w = pill_width(metrics.text_w);
    const float qty_w = pill_width(metrics.quantity_w);
    // The close button is a square cell, so its icon stays centered whatever the
    // font size works out to.
    const float close_w = metrics.closable ? metrics.label_h : 0.f;
    const float group_w = body_w + qty_w + close_w;

    if (group_w <= 0.f || metrics.label_h <= 0.f || pane_right <= 0.f) {
        // A bare line: no group, but still report a degenerate span at the
        // anchor so callers can treat left/right uniformly.
        out.left = out.right = pane_right;
        return out;
    }

    float left = 0.f;
    switch (style.align) {
        case 0:  // left
            left = 0.f;
            break;
        case 1:  // center
            left = (pane_right - group_w) * 0.5f;
            break;
        default: {  // right
            const float frac = std::clamp(style.line_length_frac, 0.f, 1.f);
            const float right = pane_right - frac * pane_right - kAxisGutter;
            left = right - group_w;
            break;
        }
    }
    // Keep the group on-screen: never past the axis, never off the left edge.
    left = std::min(left, pane_right - group_w);
    left = std::max(left, 0.f);

    const float top = y - metrics.label_h * 0.5f;
    const float bottom = top + metrics.label_h;

    float x = left;
    if (body_w > 0.f) {
        out.body = Rect{x, top, x + body_w, bottom};
        x += body_w;
    }
    if (qty_w > 0.f) {
        out.quantity = Rect{x, top, x + qty_w, bottom};
        x += qty_w;
    }
    if (close_w > 0.f) {
        out.close = Rect{x, top, x + close_w, bottom};
        x += close_w;
    }

    out.left = left;
    out.right = x;
    return out;
}

bool hits_line(float line_y, float stroke_width, float y) {
    const float band = std::max(stroke_width, 1.f) + kLineHitTolerance;
    return std::fabs(y - line_y) <= band;
}

}  // namespace vroom::price_lines
