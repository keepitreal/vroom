// Geometry for the crosshair plus button — the order-entry affordance drawn
// inside the crosshair's price badge: a "+" left of the price, the two sharing
// one pill.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_crosshair_button.cpp. Drawing lives in crosshair.cpp.

#pragma once

#include <algorithm>
#include <cstdint>

#include "vroom/vroom_chart.h"

namespace vroom::crosshair_button {

inline constexpr float kDefaultCornerRadius = 6.f;  // the plain badge's radius
inline constexpr float kDefaultHoverBoost = 1.25f;
inline constexpr float kPlusGap = 6.f;  // between the plus and the price text

// The style with every sentinel replaced by its default.
inline VroomCrosshairButtonStyle resolve(const VroomCrosshairButtonStyle& in) {
    VroomCrosshairButtonStyle s = in;
    s.corner_radius_px = in.corner_radius_px < 0.f ? kDefaultCornerRadius
                                                   : in.corner_radius_px;
    s.hover_boost = in.hover_boost > 0.f ? in.hover_boost : kDefaultHoverBoost;
    return s;
}

// The pill's corner radius for a pill `box_h` tall: never past fully rounded.
inline float corner_radius(const VroomCrosshairButtonStyle& style, float box_h) {
    return std::clamp(style.corner_radius_px, 0.f, std::max(0.f, box_h * 0.5f));
}

struct Rect {
    float left;
    float top;
    float right;
    float bottom;
};

// The merged pill for a price whose text spans `text_left..text_right`,
// centered on `cy`. The text keeps its place; the pill grows leftward by the
// plus glyph (`glyph_w`) and `gap`, with `pad_h` on both outer sides. Pass
// `text_left == text_right` and `gap == 0` for a plus-only pill.
inline Rect pill_rect(float text_left, float text_right, float cy, float box_h,
                      float glyph_w, float pad_h, float gap) {
    const float half = box_h * 0.5f;
    return Rect{text_left - gap - glyph_w - pad_h, cy - half,
                text_right + pad_h, cy + half};
}

// The plus glyph's center x inside a pill from `pill_rect`.
inline float plus_center_x(const Rect& pill, float glyph_w, float pad_h) {
    return pill.left + pad_h + glyph_w * 0.5f;
}

inline bool contains(const Rect& r, float x, float y) {
    return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

}  // namespace vroom::crosshair_button
