---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add the Fair Value Gap indicator: shaded boxes over three-candle imbalances,
where the first and third candles' wicks never overlap and leave a band of price
that was skipped.

Gaps are tracked until price trades back through them — settled by a close past
the far edge or by a wick reaching it — and a filled gap either disappears or
stops at the bar that filled it. Boxes draw behind the candles with a
per-direction fill color and opacity, an optional solid/dotted/dashed outline,
and a label that sits inside the box or out past the newest candle when the
boxes are extended.
