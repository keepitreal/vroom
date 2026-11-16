---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Footprints sit on the line in line mode, centered on the candle's close (candle mode keeps the gap above the wick), and glide between the two positions on the chart-type animation's duration and easing. A candle with both buys and sells now draws one combined badge: a bull dot and a bear dot on one disc, the side that traded first upper-left. Hovering or tapping it reports `side: 'both'` in `onFootprint`. `FootprintsStyle.gap` is deprecated and ignored.
