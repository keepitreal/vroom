---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add a `loading` prop that draws a single grey line across the plot, drifting in
a slow sine wave, while a series is being fetched. When the data lands the line
becomes the chart in two steps: it reshapes to pass through the vertical centre
of every candle about to be drawn, then fades out while those candles grow
outward from it and their colour fades up — so the chart resolves into place
rather than cutting between scenes.

The line shows only when `loading` is true and `candles` is empty, so a
background refresh leaves the chart the user is reading up and interactive.
While it's up, gestures, axis text, the price badge, the crosshair and indicator
panes are all suppressed — the line is unrelated to the asset, so nothing is
allowed to report a number off it. Style it with the new `theme.skeleton` color;
the hand-off shares `transitionMs` / `transitionEasing` with the interval
switch, and reduced motion holds the line still and skips the hand-off.
