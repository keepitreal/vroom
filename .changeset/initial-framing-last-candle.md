---
"@vroomchart/core-wasm": patch
"@vroomchart/react": patch
"react-native-vroom-chart": patch
---

Show the newest candle on first render instead of hiding it behind the price
axis.

The default framing pinned the right edge of the visible window to the last
candle's `timeMs`. A candle occupies the slot `[timeMs, timeMs + duration)` and
draws centered in it, so that put the newest candle's center half a slot past
the plot area and its whole body inside the reserved y-axis strip, which the
renderer then paints over. The last-price line and badge still rendered, so the
price was visible but the candle it came from was not — it only appeared once
the user panned right.

The window now ends at the last slot's end, so the newest candle sits flush
against the right edge of the plot area. This applies to both the
`defaultCandleWidth`-driven framing and the legacy "most recent ~80 candles"
fallback, and to `resetView` since it re-runs the same framing.
