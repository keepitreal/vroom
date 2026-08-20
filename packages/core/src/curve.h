// Monotone cubic interpolation — the math behind the line chart's optional
// corner smoothing (VROOM_FLOAT_LINE_TENSION).
//
// Deliberately monotone (Fritsch-Carlson) rather than a plain Catmull-Rom or
// cardinal spline. An unconstrained spline through the closes overshoots on a
// sharp reversal, and on a price chart that isn't just ugly — the curve dips
// below the period's low and draws a price that never traded, next to candles
// that show the real one during the candle-to-line crossfade. Limiting the
// tangents keeps every local extreme on an actual data point.
//
// Skia-free and header-only so the unit tests can cover it (see
// tests/test_curve.cpp); ma_overlay.cpp owns the walk and the path emission.

#pragma once

#include <algorithm>
#include <cmath>

namespace vroom::curve {

// The two interior control points of a cubic Bezier segment, in whatever space
// the endpoints were given in (pixels, at every call site here).
struct Controls {
    float c1x, c1y;
    float c2x, c2y;
};

// Slope of the straight line between two points. Non-increasing x yields 0,
// which keeps a degenerate span (duplicate timestamps, or the compressed
// spacing an interval morph can pass through) from producing infinities.
inline float secant(float x0, float y0, float x1, float y1) {
    const float h = x1 - x0;
    return h > 0.f ? (y1 - y0) / h : 0.f;
}

// Tangent at a point with neighboring secants `d_prev` and `d_next`.
inline float monotone_tangent(float d_prev, float d_next) {
    // Opposite signs (or a flat neighbor) mean this point is a local extreme.
    // A zero tangent is what pins the curve to it instead of sailing past.
    if (d_prev * d_next <= 0.f) return 0.f;
    // Fritsch-Carlson: holding the tangent within three times the smaller
    // neighboring secant is what makes each segment monotone, and is also why
    // the control points below can never leave the endpoints' y range.
    const float limit = 3.f * std::min(std::fabs(d_prev), std::fabs(d_next));
    return std::clamp((d_prev + d_next) * 0.5f, -limit, limit);
}

// Control points for the segment p0 -> p1, given the tangents chosen for each
// endpoint and how far to lean into them.
//
// `tension` 0 puts both tangents back on the segment's own secant, which lands
// the controls exactly on its thirds — the cubic *is* the straight line. So the
// knob is continuous at zero rather than snapping into a curve, and callers can
// interpolate it without a visible discontinuity.
inline Controls segment_controls(float x0, float y0,
                                 float x1, float y1,
                                 float m0, float m1,
                                 float tension) {
    const float d = secant(x0, y0, x1, y1);
    const float a = d + (m0 - d) * tension;
    const float b = d + (m1 - d) * tension;
    const float third = (x1 - x0) / 3.f;
    return Controls{x0 + third, y0 + a * third,
                    x1 - third, y1 - b * third};
}

}  // namespace vroom::curve
