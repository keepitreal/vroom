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
- `@shopify/react-native-skia` ≥ 1.5 — provides the Skia runtime we draw into
- `react-native-gesture-handler` ≥ 2.16 — long-press / pan / pinch recognition

## Status

Pre-alpha. The TS layer renders a placeholder chart area with working
gestures (long-press crosshair); the actual candle rendering will move to
the C++ core (`@vroom/core`) via JSI once that bridge lands. See
`cpp/README.md`.
