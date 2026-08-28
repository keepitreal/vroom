// The pixel bounding box of a drawing (see bounds_of in drawings.cpp).
//
// A host anchoring UI to a selected drawing needs one rectangle, whatever the
// shape: a box and a freehand stroke both have to yield something positionable.
// Reducing the projected anchors to their extent gives that uniformly, where the
// anchors themselves would not — a pencil stroke's `a`/`b` are just its first
// and last samples, which say nothing about how far the stroke wandered between
// them.
//
// The extent is grown by half the stroke width on every side, because a stroke
// straddles its path: a 2px line through y=100 paints 99..101. Without that the
// rectangle would clip the very pixels it is meant to describe.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_drawing_bounds.cpp.

#pragma once

#include <algorithm>
#include <cmath>

namespace vroom::drawing_bounds {

// An axis-aligned rectangle in CSS pixels, relative to the chart's top-left.
struct RectPx {
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
};

// Accumulates projected points into their extent. Points are fed one at a time
// so callers can project straight from their own storage without materializing
// an intermediate array.
struct Accumulator {
    float min_x = 0.f;
    float min_y = 0.f;
    float max_x = 0.f;
    float max_y = 0.f;
    bool  has = false;

    // Non-finite coordinates are dropped rather than propagated: a single NaN
    // would otherwise poison every comparison and hand the host an unusable
    // rectangle.
    void add(float x, float y) {
        if (!std::isfinite(x) || !std::isfinite(y)) return;
        if (!has) {
            min_x = max_x = x;
            min_y = max_y = y;
            has = true;
            return;
        }
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
    }

    // The extent grown by half of `stroke_width` on each side. A single point
    // yields a zero-extent rectangle inflated to the stroke's own footprint,
    // which is exactly what a one-sample pencil stroke paints.
    RectPx to_rect(float stroke_width) const {
        if (!has) return RectPx{};
        const float pad = std::max(stroke_width, 0.f) * 0.5f;
        return RectPx{min_x - pad, min_y - pad, (max_x - min_x) + pad * 2.f,
                      (max_y - min_y) + pad * 2.f};
    }
};

}  // namespace vroom::drawing_bounds
