---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

New theme keys style the current-price indicator's dashed line: `priceIndicatorDashLength`, `priceIndicatorDashGap`, `priceIndicatorLineWidth`, `priceIndicatorLineOpacity` and `priceIndicatorRoundCap`. Dash and gap are the lengths you see, so turning on round caps doesn't make the dashes longer. Opacity multiplies the accent color's alpha and leaves the price badge alone. The defaults match today's line (2px dashes, 2px gaps, 1px wide, full opacity, square ends), so nothing changes unless you set them.
