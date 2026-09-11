---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Let hosts pick fade or transform for interval switches

A same-asset resolution change always slot-lerped columns, which looks wrong when the two windows don't share a 1:1 pairing (e.g. a fixed lookback). Pass `intervalTransition="fade"` to fade the old scene out then the new one in; `'transform'` (the default) keeps today's reshape. Same duration and easing as `transitionMs`; reduced motion still snaps.
