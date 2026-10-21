---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

Badge text color can now be set per badge instead of only through the global `theme.badgeText`. `PriceLine.badgeTextColor` sets the text on one line's y-axis price badge and quantity pill, `crosshair.badgeTextColor` sets the text on the crosshair's badges and plus button, and `theme.priceIndicatorBadgeText` sets the text on the current-price badge. Each falls back to `theme.badgeText` when unset, so nothing changes unless you set them.

```tsx
<VroomChart
  priceLines={[{ id: 'tp', price: 2555.4, color: '#4ade80', text: 'TP', badgeTextColor: '#0b0f14' }]}
  crosshair={{ badgeColor: '#9ca3af', badgeTextColor: '#0b0f14' }}
  theme={{ priceIndicatorBadgeText: '#0b0f14' }}
/>
```
