---
'@vroomchart/strategies': minor
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Add the ORB (Opening Range Breakout) Assistant. Pass `orb={{ enabled: true }}` to draw the session's opening range, confirmed breakouts, simulated trades with stop, target and time exits, and a stats table in R. `onOrbEvent` reports each new range, breakout, entry and exit once, and `onOrbResult` returns the full result. The strategy runs as a pure, deterministic `runOrb` function from the new `@vroomchart/strategies` package, which also works without a chart. The core gains a read-only annotations layer (`setAnnotations`) of segments, boxes, markers and labels.
