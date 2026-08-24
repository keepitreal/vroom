---
'@vroomchart/core-wasm': patch
'@vroomchart/react': patch
'react-native-vroom-chart': patch
---

**Short series no longer stretch candles to fill the plot.** `defaultCandleWidth`
is a target body width, not a starting hint: if there aren't enough bars to fill
the window, empty slots sit to the left of the first candle and each bar stays
that many pixels wide. Asset switches onto a 9-candle daily series no longer
draw ~120px-wide bars. Panning that view is a no-op (candles stay right-aligned)
instead of snapping the bars to the left edge and locking.

Y-axis labels also pick their decimal places from the tick interval, so a
0.00–0.04 range no longer prints duplicate `0.01`s, and sub-cent prices
(including ~1e-7) stay distinguishable.
