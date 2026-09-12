---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add footprints: circular badges marking executed trades above the bar they filled in

Pass executed trades as `footprints` and the chart marks each one above its candle — a `+` badge in the bull color for an entry, a `−` in the bear color for an exit. Style them with `footprintsStyle`.

Timestamps are raw execution times, not bar-open times: the chart buckets every trade into whichever candle's window contains it and re-buckets on its own whenever the candles change, so one array renders correctly at every interval. At most two badges render per candle — one for its buys, one for its sells — however many fills went into each, stacked so the side whose last fill came first sits nearer the bar.

Hovering a badge (tapping, on React Native) fires `onFootprint` with every trade on that candle, the badge's on-screen position, and `pane` — the plot rect, axis strips excluded. The chart draws no tooltip of its own; that event is the hook for yours, and `pane` is what lets you pick a side for it without measuring the DOM (the element you'd measure includes the price axis, so it overstates the room beside a badge near the edge).
