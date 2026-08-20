// The pulsing ring at the line chart's tip — a phase in, a radius and two
// alphas out (VROOM_FLOAT_LINE_TIP_PULSE).
//
// Shape and timing follow TradingView's last-price animation, whose numbers are
// well-tuned: expand while the fill washes out and the edge sharpens, keep
// expanding while the edge fades, then rest. That rest is nearly half the period
// and it is what makes the ring read as a heartbeat instead of a strobe.
//
// Radii come out as multiples of the ring's start radius rather than pixels,
// because the tip dot scales with the line width and the ring has to scale with
// it (see draw_close_tip in ma_overlay.cpp).
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_tip_pulse.cpp.

#pragma once

#include <cmath>

namespace vroom::tip_pulse {

// One full expand-and-rest cycle.
constexpr float kPeriodSeconds = 2.6f;

// The ring at one instant. Alphas already account for the rest stage, so a
// caller can paint unconditionally and simply draw nothing visible.
struct Frame {
    float radius_mul;    // multiple of the ring's start radius
    float fill_alpha;
    float stroke_alpha;
};

namespace detail {

struct Stage {
    float end;  // phase this stage runs until
    float start_radius, end_radius;
    float start_fill, end_fill;
    float start_stroke, end_stroke;
};

// The fill fades as the ring grows, while the edge first *gains* alpha — that
// crossover is what gives the bloom its snap — before fading out in stage two.
constexpr Stage kStages[] = {
    {0.25f,  1.0f, 2.5f, 0.25f, 0.f, 0.40f, 0.80f},
    {0.525f, 2.5f, 3.5f, 0.f,   0.f, 0.80f, 0.f},
    {1.0f,   3.5f, 3.5f, 0.f,   0.f, 0.f,   0.f},
};

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

}  // namespace detail

// Ring state at `phase`, in cycles. Values outside [0,1) wrap, so a caller can
// hand over raw elapsed time divided by the period without normalizing.
inline Frame at(float phase) {
    const float p = phase - std::floor(phase);
    float start = 0.f;
    for (const detail::Stage& s : detail::kStages) {
        // The last stage takes anything left over, which also catches a `p` that
        // rounded up to 1 on the way in.
        if (p < s.end || s.end >= 1.f) {
            const float span = s.end - start;
            const float t = span > 0.f ? (p - start) / span : 0.f;
            return Frame{detail::lerp(s.start_radius, s.end_radius, t),
                         detail::lerp(s.start_fill, s.end_fill, t),
                         detail::lerp(s.start_stroke, s.end_stroke, t)};
        }
        start = s.end;
    }
    return Frame{1.f, 0.f, 0.f};  // unreachable: the table ends at 1
}

}  // namespace vroom::tip_pulse
