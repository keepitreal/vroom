---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

The crosshair now works over the RSI, MACD and ATR panes: the horizontal line follows the pointer into the pane and its badge reads the pane's value. `CrosshairEvent` gains `indicator` (`{ kind, value }` over a pane, else null), and over a pane `price` is the close of the candle under the line. React Native now reports `price` too.
