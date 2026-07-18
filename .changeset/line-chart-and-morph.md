---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Line chart mode with an animated candle↔line transition (web + React Native):

- **`chartType` prop** (`'candles' | 'line'`) — `'line'` hides the candles and draws
  a polyline through each candle's close. Volume, indicators (RSI/MACD/MAs/VWAP),
  crosshair, and drawings all keep rendering. Style the line with `theme.lineColor`
  and `theme.lineWidth`.
- **Animated transition** — switching `chartType` smoothly morphs between the two:
  each candle collapses toward its close as the connecting line fades in (and the
  reverse). Control the duration with the new **`transitionMs`** prop (default ~300 ms;
  `0` snaps instantly). Honors `prefers-reduced-motion` on web (a plain cross-fade with
  no vertical motion) and retargets smoothly if toggled mid-transition.

Both are additive and default to the previous behavior (candlesticks).
