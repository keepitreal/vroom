// The travelling sine the loading line rides while a series is being fetched.
//
// A position across the plot and an elapsed time in, a vertical offset out.
// Phase runs as (k*x - w*t), so a crest sits at a larger x as time advances —
// the wave moves left to right, the direction the series grows.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_loading_wave.cpp.

#pragma once

#include <cmath>

namespace vroom::loading_wave {

constexpr float kTwoPi = 6.283185307179586f;

// Crests visible at once. Above about two the line starts reading as data
// rather than as a placeholder; below one it barely looks like a wave.
constexpr float kCyclesAcrossWidth = 1.5f;

// How fast the phase advances. The line completes a cycle every 2*pi/w ~= 3.9s
// and a crest crosses the full width in that over kCyclesAcrossWidth ~= 2.6s —
// a drift rather than a ripple, which is the point: this runs under a chart
// that has nothing to say yet.
constexpr float kTemporalRadPerSecond = 1.6f;

// One cycle in time. Callers wrap elapsed time by this so a chart left loading
// for minutes doesn't lose float precision on the phase.
constexpr float kPeriodSeconds = kTwoPi / kTemporalRadPerSecond;

// Peak displacement from the centreline, as a fraction of the price pane's
// height. The wave spans twice this, so 0.15 puts it across the middle ~30%
// and keeps the crests well clear of the pane edges.
constexpr float kAmplitudeFrac = 0.15f;

// Offset at `x_frac` (0 = left edge of the plot, 1 = right) and `elapsed_s`,
// in -1 .. 1. Multiply by kAmplitudeFrac and the pane height for pixels.
// Time outside one period is fine — sin wraps on its own.
inline float at(float elapsed_s, float x_frac) {
    return std::sin(kCyclesAcrossWidth * kTwoPi * x_frac -
                    kTemporalRadPerSecond * elapsed_s);
}

// The wave's y as a fraction of the pane height (0 = pane top, 1 = bottom),
// centred on the pane. This is the form the line capture stores, so a resize
// mid-morph rescales instead of stranding the curve at old pixels.
inline float y_frac(float elapsed_s, float x_frac) {
    return 0.5f + kAmplitudeFrac * at(elapsed_s, x_frac);
}

}  // namespace vroom::loading_wave
