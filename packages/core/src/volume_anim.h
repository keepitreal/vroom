// Volume-bar collapse timing — pure functions, no Skia, no state.
//
// Split out from volume.cpp so the stagger arithmetic unit-tests without a Skia
// checkout, and so the one piece of easing the core owns lives somewhere
// obvious.
//
// Unlike the candle↔line and interval morphs, the host can't pre-ease this
// animation: every bar runs on its own window, so easing has to be applied per
// bar, after the window split. Feed bar_collapse the raw linear progress.

#pragma once

#include <cstdint>

namespace vroom::volume_anim {

// Shortest window any bar gets, as a fraction of the timeline. Without a floor a
// near-zero bar would collapse inside a single frame and read as a pop rather
// than a fall. Clamping only moves a start earlier, so every bar still lands
// together.
constexpr float kMinWindow = 0.2f;

// Linear progress (0..1) mapped through a VroomEasing curve. Mirrors ease() in
// packages/react/src/easing.ts; unknown kinds fall back to smoothstep, matching
// it. Progress outside 0..1 is clamped.
float ease(int32_t kind, float p);

// How far a single bar has collapsed at global linear progress `t`, from 0
// (untouched, full height) to 1 (flat).
//
// `frac` is the bar's height as a fraction of the tallest bar in view, and
// doubles as the length of its window: a bar falls over the last `frac` of the
// timeline. So the tallest bar starts immediately and takes the whole
// transition, shorter bars start progressively later, and everything reaches
// zero at t = 1 — the "tallest first, all land together" cascade.
//
// Symmetric in `t`, so driving it backwards (1 -> 0) plays the reveal: the
// shortest bar rises first and the tallest arrives last.
float bar_collapse(float frac, float t, int32_t easing);

}  // namespace vroom::volume_anim
