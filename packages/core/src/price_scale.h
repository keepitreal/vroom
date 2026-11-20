// Who owns the price range: the auto-fit, the eased autoscale, or the user.
// These are the rules for which gestures may take it over and what a reset
// hands it back to.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_price_scale.cpp. The state lives on VroomChart (chart.h) and the
// facade applies these rules to it (chart_facade.cpp).

#pragma once

#include "viewport.h"

namespace vroom::price_scale {

// VroomChart's autoscale_y, axes_user_adjusted and price_bounds_manual.
struct Flags {
    bool autoscale_y = false;
    bool user_adjusted = false;
    bool manual = false;
};

// Whether the eased auto-fit is in charge of the y-range right now.
constexpr bool autoscale_engaged(Flags f) {
    return f.autoscale_y && !f.user_adjusted && !f.manual;
}

// Whether the vertical part of a plot drag moves price. Not while autoscale is
// engaged: a finger never drags perfectly level, and taking the scale manual
// on that jitter would end autoscale on nearly every pan.
constexpr bool translate_moves_price(Flags f) { return !autoscale_engaged(f); }

// Whether a zoom's y factor scales price. Under autoscaleY only once a
// price-axis drag has taken the scale over; a zoom never takes it over itself.
constexpr bool zoom_scales_price(Flags f) { return !f.autoscale_y || f.manual; }

// Whether a zoom or an axis scale is recorded as the user's own framing
// (axes_user_adjusted). Not under autoscaleY: a takeover there is undone by
// reset_price_scale, and the record would keep autoscale from re-engaging
// until the next reset_view.
constexpr bool records_user_adjustment(Flags f) { return !f.autoscale_y; }

// What reset_price_scale does once it has dropped the manual bounds.
enum class Handback {
    kNone,  // nothing was manual
    kFit,   // back to the instant auto-fit
    kEase,  // back to autoscale, easing home from the bounds on screen
    kSnap,  // back to autoscale, landing on the fit (reduced motion)
};

constexpr Handback handback(Flags f, bool animate) {
    if (!f.manual) return Handback::kNone;
    f.manual = false;
    if (!autoscale_engaged(f)) return Handback::kFit;
    return animate ? Handback::kEase : Handback::kSnap;
}

// One frame of the eased range (see ease_price_bounds). `*handing_back` is set
// by a kEase hand-back, whose starting range can sit inside the candles'
// low..high: the user had zoomed in, or slid the range away. Holding that range
// to the envelope, as the ease otherwise does, would jump it there on the first
// frame, so it eases free until it contains the envelope or has landed.
inline PriceBounds ease(const PriceBounds& display, const PriceBounds& target,
                        const PriceBounds& envelope, float dt_s, bool animate,
                        bool* handing_back) {
    const PriceBounds unbounded{1.0, 0.0};
    const PriceBounds out = ease_price_bounds(
        display, target, *handing_back ? unbounded : envelope, dt_s, animate);
    const bool contains = out.min <= envelope.min && out.max >= envelope.max;
    const bool landed = out.min == target.min && out.max == target.max;
    if (contains || landed) *handing_back = false;
    return out;
}

}  // namespace vroom::price_scale
