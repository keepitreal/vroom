# vroom 🏎️💨

Lightning fast, advanced candlestick charting library for native iOS/Android and React Native. Core charting library built with Skia and C++ for optimum rendering performance across all mobile platforms.

## React Native

### Install

```sh
npm install react-native-vroom-chart
```

Peer dependencies (install if you don't already have them):

```sh
npm install @shopify/react-native-skia react-native-gesture-handler react-native-reanimated
```

### Usage

```tsx
import { VroomChart, type Candle } from "react-native-vroom-chart";

const candles: Candle[] = [
  {
    timeMs: 1700000000000,
    open: 100,
    high: 105,
    low: 98,
    close: 103,
    volume: 1200,
  },
  // ...
];

export function Chart() {
  return <VroomChart candles={candles} width={360} height={360} />;
}
```

Pan to scroll, pinch to zoom, and drag the price/time axes to rescale them.

### Props

| Prop               | Type                          | Description                                                                                                          |
| ------------------ | ----------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `candles`          | `Candle[]`                    | Required. OHLCV data (`timeMs`, `open`, `high`, `low`, `close`, `volume`).                                           |
| `width`            | `number`                      | Chart width in px. Default `360`.                                                                                    |
| `height`           | `number`                      | Chart height in px. Default `240`.                                                                                   |
| `visibleRange`     | `{ startMs, endMs }`          | Time window to render. Omit to fit all candles.                                                                      |
| `theme`            | `VroomTheme`                  | Color overrides (`background`, `bull`, `bear`, `wick`, `grid`, `axisText`, `crosshair`, `tooltipBg`, `tooltipText`). |
| `onCrosshair`      | `(e: CrosshairEvent) => void` | Fires as the crosshair moves (`active`, `timeMs`, `price`).                                                          |
| `onViewportChange` | `(startMs, endMs) => void`    | Fires when the visible time range changes via gesture.                                                               |

### iOS & Android (coming soon)

Standalone Swift and Kotlin packages with idiomatic native APIs are planned. For now, all
platforms are consumed through the React Native package above.

## Layout

- `packages/core/` — platform-agnostic C++ engine (Skia-based). Public C facade.
- `packages/react-native/` — RN npm package. JSI bridge + iOS/Android shims around the core.
- `examples/react-native/` — Expo dev-client test bench app.
- `third_party/skia/` — fetched prebuilts (gitignored).
- `scripts/` — build helpers.

Future:

- `packages/ios/` — standalone SwiftPM/CocoaPod with idiomatic Swift API.
- `packages/android/` — standalone AAR with idiomatic Kotlin API.

## Dev quickstart

```sh
pnpm install
pnpm example          # runs the RN test bench
```

## Architecture

See `docs/` (TODO). Short version: `packages/core/` exposes a thin `extern "C"` API
(`vroom_chart.h`). Each distribution wraps that facade — RN via JSI, iOS via Swift, Android
via JNI. The core owns all Skia drawing; consumers only provide an `SkCanvas` and forward
gesture events.
