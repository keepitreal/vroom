---
'@vroomchart/core-wasm': patch
'@vroomchart/react': patch
'react-native-vroom-chart': patch
---

Keep candle width across timeframe switches in three cases that crushed or blew up the candles:

- A switch that goes through the `loading` line now re-anchors from the view the chart had before loading (a different asset still resets), instead of landing the new interval in a window measured in the old interval's milliseconds.
- The core infers the candle period from the median of the leading intervals, so a series starting with a gap or a repeated timestamp no longer draws candles at the wrong width (or keeps the previous interval's width).
- Web: with a controlled `visibleRange` the host doesn't move during the switch, the window is re-anchored and reported through `onViewportChange`.
