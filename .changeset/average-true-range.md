---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add the Average True Range indicator through a new `atr` prop. ATR renders in
its own pane below the candles, measuring volatility in price units: True Range
is the widest of a bar's own high-low span and the two gaps from its extremes to
the previous close, so an overnight move the bar's range misses still counts.

`smoothing` chooses how that series is averaged — `'rma'` (Wilder's running
average, the default every published ATR level assumes), `'sma'`, or `'ema'`.
The pane fits `0..peak` anchored at its bottom edge, since ATR is strictly
positive and unbounded, and labels the peak in price units.

The pane stack now holds three panes rather than two, so RSI, MACD, and ATR can
all be on at once; they stack in enable order with the most recent at the
bottom, and the y-axis strip beside each one zooms that pane.
