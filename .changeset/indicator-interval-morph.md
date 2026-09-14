---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Animate indicator lines through timeframe switches. Previously only the candles
and the close line reshaped when the resolution changed — every moving average,
VWAP, Bollinger band, Ichimoku line, and the RSI, MACD and ATR panes popped
straight to the new resolution's values on the first frame, so the indicators
visibly detached from the price series they describe.

Each series now captures its outgoing shape before the switch and eases into
the new one, the same slot pairing the candles use. That covers the Bollinger
fill and Ichimoku cloud, which are stitched from the same interpolated vertices
as their edge lines rather than detaching from them, and the MACD histogram,
whose bars reshape the way candle bodies do. The ATR pane's peak label eases
between the two fits alongside its curve.

With `intervalTransition: 'fade'`, the indicator panes used to disappear
outright — background, divider and all — for the first half of the transition.
They now fade out with the price series and fade back in with the new data.

No API change: `intervalTransition` already selected the mode.
