// The travelling pulse that runs through the loading skeleton's bars — an
// elapsed time and a bar index in, an alpha and a height scale out.
//
// One sine drives both outputs, in phase, so a crest is simultaneously the
// tallest and the most opaque bar and a trough the shortest and faintest. That
// coupling is the whole trick: two independent effects read as two effects,
// while one shared phase reads as a single pulse moving through the series.
//
// Phase decreases with bar index, so crests advance toward higher indices —
// left to right, the direction the series grows.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_loading_wave.cpp.

#pragma once

#include <cmath>

namespace vroom::loading_wave {

// Angular frequency in time and in space. The ratio is the phase velocity:
// 6 / 0.4 = 15 bars per second, with a wavelength of 2*pi / 0.4 ~= 15.7 bars —
// so roughly one full wave is on screen at a time and it crosses a 60-bar
// window in about four seconds.
constexpr float kTemporalRadPerSecond = 6.f;
constexpr float kSpatialRadPerBar = 0.4f;

// One cycle in time. Callers wrap elapsed time by this so a chart left loading
// for minutes doesn't lose float precision on the phase.
constexpr float kPeriodSeconds = 6.283185307179586f / kTemporalRadPerSecond;

// Alpha floor and height scale at the extremes of the sine. The floor keeps a
// trough legible rather than blinking out, and a scale that dips below 1 as far
// as it rises above it keeps the series' average silhouette unchanged.
constexpr float kMinAlpha = 0.35f;
constexpr float kMaxAlpha = 1.f;
constexpr float kMinScale = 0.5f;
constexpr float kMaxScale = 1.5f;

struct Frame {
    float alpha;  // kMinAlpha .. kMaxAlpha
    float scale;  // kMinScale .. kMaxScale, about each bar's own midpoint
};

// The pulse at `elapsed_s` for the bar at absolute index `index`. Time outside
// one period is fine — sin wraps on its own — so a caller can hand over raw
// elapsed seconds.
inline Frame at(float elapsed_s, int index) {
    const float w = std::sin(kTemporalRadPerSecond * elapsed_s -
                             kSpatialRadPerBar * static_cast<float>(index));
    // sin maps to 0..1 first so both outputs read as a plain lerp across their
    // range, which is what keeps them provably in bounds.
    const float u = 0.5f + 0.5f * w;
    return Frame{kMinAlpha + (kMaxAlpha - kMinAlpha) * u,
                 kMinScale + (kMaxScale - kMinScale) * u};
}

}  // namespace vroom::loading_wave
