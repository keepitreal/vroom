// Geometry for the crosshair plus button — the order-entry affordance that
// sits on the crosshair's horizontal line, directly left of the price badge.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_crosshair_button.cpp. Drawing lives in crosshair.cpp.

#pragma once

#include <algorithm>
#include <cstdint>

#include "vroom/vroom_chart.h"

namespace vroom::crosshair_button {

inline constexpr float kDefaultSize = 20.f;
inline constexpr float kMinSize = 12.f;
inline constexpr float kMaxSize = 48.f;
inline constexpr float kDefaultCornerRadius = 4.f;
inline constexpr float kDefaultIconStroke = 1.5f;
inline constexpr float kDefaultGap = 4.f;
inline constexpr float kDefaultHoverBoost = 1.25f;

// The style with every sentinel replaced by its default and every value
// clamped into range. Colors stay 0 = "inherit from the theme"; the draw path
// resolves those, since only it has the theme.
inline VroomCrosshairButtonStyle resolve(const VroomCrosshairButtonStyle& in) {
    VroomCrosshairButtonStyle s = in;
    s.size_px = in.size_px > 0.f ? std::clamp(in.size_px, kMinSize, kMaxSize)
                                 : kDefaultSize;
    s.corner_radius_px = in.corner_radius_px < 0.f
                             ? kDefaultCornerRadius
                             : in.corner_radius_px;
    s.corner_radius_px = std::min(s.corner_radius_px, s.size_px * 0.5f);
    s.icon_stroke_px = in.icon_stroke_px > 0.f ? in.icon_stroke_px
                                               : kDefaultIconStroke;
    s.gap_px = in.gap_px < 0.f ? kDefaultGap : in.gap_px;
    s.hover_boost = in.hover_boost > 0.f ? in.hover_boost : kDefaultHoverBoost;
    return s;
}

struct Rect {
    float left;
    float top;
    float right;
    float bottom;
};

// The button for a crosshair at `cy`. `anchor_right` is where the button's
// right side meets: the price badge's left edge, or the plot's right edge when
// the badge isn't drawn. `pane_top` / `pane_bottom` bound the price pane; the
// button stays fully inside it even when the line sits near an edge.
inline Rect button_rect(float anchor_right, float cy, float pane_top,
                        float pane_bottom,
                        const VroomCrosshairButtonStyle& style) {
    const float size = style.size_px;
    const float right = anchor_right - style.gap_px;
    float top = cy - size * 0.5f;
    const float max_top = std::max(pane_top, pane_bottom - size);
    top = std::clamp(top, pane_top, max_top);
    return Rect{right - size, top, right, top + size};
}

inline bool contains(const Rect& r, float x, float y) {
    return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

}  // namespace vroom::crosshair_button
