---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Volume bars are now configurable through a `volume` prop, alongside the other
indicators:

```tsx
<VroomChart
  candles={candles}
  volume={{ enabled: true, opacity: 0.35, height: 0.25, radius: 2 }}
/>
```

- `enabled` toggles the bars. It defaults to `true`, so omitting the prop leaves
  existing charts unchanged; pass `{ enabled: false }` to hide them.
- `opacity` (default 0.5) sets how much quieter the bars read than the candles.
- `height` (default 0.2) is the tallest bar as a fraction of the price pane.
  It's a ceiling rather than a reserved strip, matching TradingView's built-in
  volume: raising it lets the bars reach further up over the candles instead of
  compressing them.
- `radius`, `upColor` and `downColor` restyle the bars, defaulting to
  `theme.volumeRadius` and `theme.accentBull` / `theme.accentBear`.

`theme.volumeRadius` is deprecated in favor of `volume.radius`, which sits with
the rest of the volume styling. It still applies when `volume.radius` is omitted.
