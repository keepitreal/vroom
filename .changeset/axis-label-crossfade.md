---
"@vroomchart/core-wasm": patch
"@vroomchart/react": patch
"react-native-vroom-chart": patch
---

Fix axis label fades never animating on web, and swap the axes with a fade-out
then fade-in across a timeframe switch instead of letting them snap.

The label fade machinery has been in place for a while, but on web it never ran:
`vroom_chart_draw` — the entry point the WASM build uses — called `draw_chart`
directly, while the per-frame `dt` was computed in `rebuild_chart_picture`, which
only the React Native SkPicture path goes through. `dt` therefore sat at 0
forever, and 0 is the fade updaters' "snap to target" signal. The clock now lives
in `draw_chart`, so both hosts tick it.

A second snap hid behind the first: a gap over 100ms clamped `dt` to 0, the very
thing the surrounding comment said the clamp was there to prevent. Since a
discrete event (timeframe switch, asset switch) always starts from an idle chart,
its first frame took the snap path and finished every fade at once. Long gaps now
clamp to a nominal frame instead.

An interval morph then hands the axes to a two-phase envelope: the pre-switch
ticks fade out over the first half of the morph, the new ones fade in over the
second. Nothing translates. Both halves are clocked off the morph's eased
progress, so the axes take `transitionMs`, follow `transitionEasing`, and land
with the candles. While fading out, the axes stay laid out against the captured
pre-switch scale and window rather than the new ones, so a label is never drawn
anywhere it wasn't already; the tick set is swapped at the midpoint, when the axis
is fully transparent.
