// The pulsing ring at the line chart's tip — a phase in, a radius and an alpha
// out (VROOM_FLOAT_LINE_TIP_PULSE).
//
// Shaped like a live-price knob and timed like Liveline's live dot: a stroked
// ring starts with its inner edge on the dot, grows outward by twice its width
// while it fades out, then rests until the next beat. That rest is what makes
// the ring read as a heartbeat instead of a strobe.
//
// Sizes come out as multiples of the dot radius rather than pixels, because the
// tip dot scales with the line width and the ring has to scale with it (see
// draw_close_tip in ma_overlay.cpp).
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_tip_pulse.cpp.

#pragma once

#include <algorithm>
#include <cmath>

namespace vroom::tip_pulse {

// One full expand-and-rest cycle.
constexpr float kPeriodSeconds = 1.5f;

// How long the ring is visible at the start of each cycle.
constexpr float kRingSeconds = 0.9f;

// Ring width as a multiple of the dot radius: a 6pt dot gets a 5pt ring.
constexpr float kWidthMul = 5.f / 6.f;

// Default for VROOM_FLOAT_LINE_TIP_PULSE_OPACITY, the alpha the ring starts at.
// Liveline's live-dot pulse starts at 0.35.
constexpr float kDefaultStartAlpha = 0.4f;

// The ring at one instant. Alpha already accounts for the rest, so a caller can
// paint unconditionally and simply draw nothing visible.
struct Frame {
    float radius_mul;  // the ring's centerline, as a multiple of the dot radius
    float alpha;
};

// Ring state at `phase`, in cycles. Values outside [0,1) wrap, so a caller can
// hand over raw elapsed time divided by the period without normalizing.
// `start_alpha` is clamped to [0,1].
inline Frame at(float phase, float start_alpha = kDefaultStartAlpha) {
    const float p = phase - std::floor(phase);
    const float t = std::min(1.f, p * kPeriodSeconds / kRingSeconds);
    return Frame{1.f + kWidthMul / 2.f + t * 2.f * kWidthMul,
                 std::clamp(start_alpha, 0.f, 1.f) * (1.f - t)};
}

}  // namespace vroom::tip_pulse
