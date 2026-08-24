# react-native-vroom-chart

React Native component for the vroom Skia candlestick chart.

```tsx
import { VroomChart } from 'react-native-vroom-chart';

<VroomChart
  candles={candles}
  width={width}
  height={240}
  onCrosshair={(e) => console.log(e)}
/>
```

## Peer dependencies

- `react-native` ≥ 0.76
- `@shopify/react-native-skia` ≥ 2.11.0 — provides the Skia runtime we draw into. vroom links RN-Skia's native internals (`cpp/api`) and tracks that package closely; those headers have no semver guarantee.

- `react-native-gesture-handler` ≥ 2.16 — long-press / pan / pinch recognition
- `react-native-reanimated` ≥ 4.0 — gesture worklets
- `react-native-worklets` ≥ 0.7.0 — required by Skia 2.11's Reanimated integration on native

## Status

Pre-alpha. The TS layer renders a placeholder chart area with working
gestures (long-press crosshair); the actual candle rendering will move to
the C++ core (`@vroomchart/core`) via JSI once that bridge lands. See
`cpp/README.md`.
