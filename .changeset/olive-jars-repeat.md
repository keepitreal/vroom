---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add `theme.badgeText`, controlling the text drawn on filled badges.

The current-price indicator's label was hardcoded white, along with the
crosshair's price and time badges and the price-line pills. All four now read
one theme color, so a light theme can make them legible instead of leaving
white text on a pale fill.

```tsx
<VroomChart theme={{ badgeText: '#1b1f24' }} />
```

Defaults to white, so existing themes render exactly as before.

**Breaking (core-wasm only):** the `ColorKey` enum drops `TooltipBg` and
`TooltipText`. Both were vestigial — they had palette defaults but were never
read by any draw call and were not reachable through `VroomTheme`. Slot 8 is
now `ColorKey.BadgeText`; slot 7 is `Reserved7`, held empty so the indices
after it don't shift, since the theme maps hardcode them.
