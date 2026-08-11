---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Animate the candles themselves across a timeframe switch: each one slides and
stretches into the shape of its counterpart in the new interval instead of the
whole series jumping.

Because the time axis preserves each candle's pixel slot across a switch, the
k-th candle from the right edge occupies the same columns before and after — so
only y has to move. The core now captures the outgoing wick and body geometry
(as price-band fractions, which keeps frame 0 pixel-identical to the pre-switch
frame and survives a resize mid-animation) and interpolates each slot toward its
new counterpart. A candle that changes direction crossfades its color; a slot
present on only one side fades rather than pops.

Add `transitionEasing` (`'linear' | 'ease-in' | 'ease-out' | 'ease-in-out'`,
default `'ease-in-out'`) alongside `transitionMs`, which now covers both this
animation and the candle↔line switch. `transitionMs={0}` or an OS reduced-motion
preference snaps as before, skipping the capture entirely.

Two new handle methods back this, both web-only for now:
`beginIntervalMorph` captures the geometry and `setIntervalMorph` advances the
animation. Volume bars, overlays and indicators still snap.
