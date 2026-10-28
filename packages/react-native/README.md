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

- `react-native` ≥ 0.82, New Architecture enabled — required by Gesture Handler 3; the chart installs its JSI bindings through the TurboModule JSI-bindings hooks, which have no legacy-bridge equivalent
- React Native Skia — provides the Skia runtime we draw into. Install **one** of `@shopify/react-native-skia` ≥ 2.11.0 or `react-native-skia` ≥ 2.15.0 (the same library under its new name). Skia 3.x is Graphite-only and needs Android `minSdkVersion` 26. vroom detects which package is installed at build time (podspec, Gradle) and at runtime (`src/skia.ts`); never install both, since that registers two copies of the native module. vroom links RN-Skia's native internals (`cpp/api`) and tracks that package closely; those headers have no semver guarantee.

- `react-native-gesture-handler` ≥ 3.4.0 — long-press / pan / pinch / tap recognition through the v3 gesture hooks; v2 is no longer supported
- `react-native-reanimated` ≥ 4.0 — gesture worklets
- `react-native-worklets` ≥ 0.7.0 — required by Skia 2.11's Reanimated integration on native

## Status

Pre-alpha. The TS layer renders a placeholder chart area with working
gestures (long-press crosshair); the actual candle rendering will move to
the C++ core (`@vroomchart/core`) via JSI once that bridge lands. See
`cpp/README.md`.
