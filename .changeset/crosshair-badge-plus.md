---
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
---

The crosshair plus button now draws inside the crosshair price badge: one pill with the "+" left of the price, and the whole pill is clickable. `CrosshairButtonConfig` is down to `enabled`, `cornerRadius` (now clamped to half the badge height, so large values draw a fully rounded pill; default 6) and `hoverBoost`. **Breaking:** `size`, `background`, `iconColor`, `iconStrokeWidth`, `ring`, `ringColor` and `gap` are removed — the pill takes its colors from the theme (`crosshairTarget` / `badgeText`). `CrosshairButtonEvent.button` is now the pill's rect.

New `theme.badgeFontSize` (10–14px) sizes the text on every filled badge — the crosshair price and time badges, the current-price badge, and price-line pills unless `priceLinesStyle.fontSize` is set. Omit it (or pass 0) to keep following `axisFontSize`.
