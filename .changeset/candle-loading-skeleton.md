---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add a `loading` prop that draws a candle skeleton while a series is being
fetched: a travelling wave of grey placeholder bars with matching volume,
gridlines and axis pills. When the data lands the placeholder bars morph into
the real ones and their grey blends into each bar's bull/bear color, so the
chart fills in rather than cutting between scenes.

The skeleton shows only when `loading` is true and `candles` is empty, so a
background refresh leaves the chart the user is reading up and interactive.
While it's up, gestures, axis text, the price badge, the crosshair and indicator
panes are all suppressed — the placeholder walk is unrelated to the asset, so
nothing is allowed to report a number off it. Style it with the new
`theme.skeleton` color, and reduced motion holds it still.
