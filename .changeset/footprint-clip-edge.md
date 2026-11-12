---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
'@vroomchart/react': patch
---

Footprint badges are clipped where the price series is, not at the candle area's edge: they follow the line into the gutter a hidden y-axis frees up and fade out under the axis with `theme.yAxisFade`, instead of being sliced while the series keeps going.
