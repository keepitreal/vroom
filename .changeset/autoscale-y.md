---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

Add an `autoscaleY` prop. While it's on and the user hasn't zoomed or scaled either axis, the price axis follows the visible candles during a pan, easing quickly so the high and low always stay in the plot. The vertical part of a one-finger drag is ignored so the fit isn't broken by jitter. Zooming, a pinch, or dragging either axis turns it off until the view is reset or the `seriesKey` changes. A timeframe switch on the same key keeps the current state. The ease is skipped when the OS asks for reduced motion. `@vroomchart/core-wasm` exposes the same mode as `VroomChartHandle.setAutoscaleY(enabled, animate)`.
