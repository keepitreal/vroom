---
"@vroomchart/core-wasm": minor
"react-native-vroom-chart": minor
"@vroomchart/react": minor
---

Toggling `volume.enabled` now animates the bars instead of snapping them.

Turning volume off sinks the bars out of view in a staggered cascade: each bar
falls over the last `height / tallestHeight` of the transition, so the tallest
starts immediately and takes the whole window, shorter bars start progressively
later, and every bar reaches the axis at the same moment. Turning volume back on
plays the same timeline backwards, which brings the shortest bars home first and
lands the tallest last.

Because each bar runs on its own window, the easing has to be applied per bar
rather than to the animation as a whole — the host feeds the core linear progress
and the core applies `transitionEasing` inside each window. A bar too short to
warrant its own window gets a floor so it falls rather than pops, and toggling
mid-animation reverses from wherever the bars are instead of jumping.

`transitionMs` and `transitionEasing` drive it, matching the candle↔line and
timeframe transitions; `transitionMs: 0` or `prefers-reduced-motion` still snaps.
No API change — `volume.enabled` is unchanged.
