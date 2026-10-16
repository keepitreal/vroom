---
'react-native-vroom-chart': minor
---

The current-price line's style and color now blend across a candle↔line switch. When `priceIndicator*` theme keys change in the same render as `chartType`, the dash, gap, width and opacity ease in step with the chart's morph and the caps switch halfway, instead of snapping while the chart morphs around them. The dashes stay anchored to the price pill throughout. The line and price pill color eases the same way, so swapping `accentBull` / `accentBear` with the chart type (say, a red period color in line mode for a green latest candle) fades from one to the other. A fresh mount, `transitionMs: 0` and reduced motion still snap.
