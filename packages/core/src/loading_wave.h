// The idle animation the loading line rides while a series is being fetched.
// A position across the plot and an elapsed time in; a vertical offset and a
// brightness out.
//
// Three sines, not one. A single sine is too regular to read as anything but a
// test pattern — the eye finds the repeat immediately and the line stops
// looking like it's waiting for something. Summing three frequencies that
// aren't multiples of each other, each drifting at its own rate, gives a curve
// that keeps rearranging itself.
//
// The other half of the effect is restraint: a small amplitude and a faint,
// slowly breathing opacity. The line is a hint that the chart is alive, not a
// feature competing with the data about to replace it.
//
// Approach adapted from Liveline (github.com/benjitaylor/liveline, MIT), whose
// loading state solves the same problem well.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_loading_wave.cpp.

#pragma once

#include <cmath>

namespace vroom::loading_wave {

constexpr float kTwoPi = 6.283185307179586f;

// One component of the curve.
struct Harmonic {
    float freq;    // radians across the full plot width
    float weight;  // share of the amplitude
    float drift;   // multiple of the base phase speed
};

// Frequencies deliberately not integer multiples of one another: harmonics
// would lock into one repeating shape, which is the thing being avoided. The
// drifts are all multiples of a quarter only so the whole curve has an exact
// period to wrap the phase clock on — see kPeriodSeconds.
//
// Four components, weighted toward the middle of the range, because the curve
// has to sit convincingly next to candlesticks. A gentler two-or-three-crest
// undulation suits a smooth live line chart, but against minute-resolution
// price action it reads as decorative, and it makes the hand-off a real change
// of shape rather than the same curve coming into focus.
constexpr Harmonic kHarmonics[] = {
    {14.0f, 0.40f, 1.00f},  // carrier — a little over two crests per screen
    {25.0f, 0.28f, 1.25f},  // ripple, running ahead
    {45.0f, 0.16f, 1.50f},  // fine texture, ahead again
    {6.0f, 0.16f, 0.75f},   // a slow swell, running behind
};
constexpr int kHarmonicCount = 4;

// Base phase speed. Slow on purpose: this plays under a chart that has nothing
// to say yet, and anything quicker reads as activity.
constexpr float kDriftRadPerSecond = 1.0f;

// One full cycle of the *whole* curve. The drifts differ by quarters, so every
// component returns to its starting phase only after four base cycles — which
// is also why the shape takes ~25s to come back around. Callers wrap elapsed
// time by this so a chart left loading doesn't lose float precision on the
// phase, and wrapping here is seamless because it is a true period.
constexpr float kPeriodSeconds = 4.f * kTwoPi / kDriftRadPerSecond;

// Peak displacement from the centreline, as a fraction of the pane height. The
// components rarely peak together, so the curve typically occupies well under
// this — it is a bound, not the height it looks.
constexpr float kAmplitudeFrac = 0.10f;

// Breathing opacity. Dim enough to read as the chart's own line waiting rather
// than as data, but no dimmer: vroom's default plot is a large near-black
// surface, and much below this the line stops looking deliberate and starts
// looking like a rendering artifact.
constexpr float kBreathMin = 0.30f;
constexpr float kBreathMax = 0.55f;
// Ten breaths per curve cycle, so the breath wraps with the phase (~2.5s each)
// and never syncs with the drift into one combined pulse.
constexpr float kBreathCyclesPerPeriod = 10.f;

// Offset at `x_frac` (0 = left edge of the plot, 1 = right) and `elapsed_s`, in
// -1 .. 1. The weights sum to 1, so that range is tight. Phase runs as
// (k*x - w*t), which walks the crests rightward — the direction the series
// grows. Time outside one period is fine; sin wraps on its own.
inline float at(float elapsed_s, float x_frac) {
    const float base = kDriftRadPerSecond * elapsed_s;
    float sum = 0.f;
    for (int i = 0; i < kHarmonicCount; ++i) {
        const Harmonic& h = kHarmonics[i];
        sum += h.weight * std::sin(h.freq * x_frac - h.drift * base);
    }
    return sum;
}

// The curve's y as a fraction of the pane height (0 = pane top, 1 = bottom),
// centred on the pane. This is the form the line capture stores, so a resize
// mid-morph rescales instead of stranding the curve at old pixels.
inline float y_frac(float elapsed_s, float x_frac) {
    return 0.5f + kAmplitudeFrac * at(elapsed_s, x_frac);
}

// The line's opacity at `elapsed_s`, in kBreathMin .. kBreathMax.
inline float breath(float elapsed_s) {
    const float mid = (kBreathMin + kBreathMax) * 0.5f;
    const float half = (kBreathMax - kBreathMin) * 0.5f;
    return mid + half * std::sin(kTwoPi * kBreathCyclesPerPeriod * elapsed_s /
                                 kPeriodSeconds);
}

// The tightest component in the table above.
constexpr float max_freq() {
    float m = 0.f;
    for (int i = 0; i < kHarmonicCount; ++i) {
        if (kHarmonics[i].freq > m) m = kHarmonics[i].freq;
    }
    return m;
}

// Vertices per cycle of that component needed to keep it from aliasing into a
// shape the curve doesn't have. Ten is comfortably past the point where the
// spline through the samples is indistinguishable from the real curve.
constexpr float kSamplesPerCycle = 10.f;

// How many vertices to sample the curve at across a plot `width_px` wide.
//
// The floor comes from the harmonic table rather than a literal, so retuning
// the curve can't silently undersample it — a narrow chart still gets enough
// vertices to resolve the finest component, it just spaces them more tightly.
//
// Both the idle curve and the morph capture size themselves through here, so
// the two are always sampled the same way and the hand-off has no shape
// discontinuity to hide.
inline int sample_count(float width_px) {
    const int floor_n =
        static_cast<int>(max_freq() / kTwoPi * kSamplesPerCycle) + 1;
    const int n = static_cast<int>(width_px / 8.f);
    const int ceil_n = 4 * floor_n;
    return n < floor_n ? floor_n : (n > ceil_n ? ceil_n : n);
}

}  // namespace vroom::loading_wave
