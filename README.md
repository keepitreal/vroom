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
import { View } from "react-native";
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
  // Fills its parent by default — size it with `style` (flex / aspectRatio /
  // absolute fill). Pass explicit `width`/`height` only to override.
  return (
    <View style={{ flex: 1 }}>
      <VroomChart candles={candles} style={{ flex: 1 }} />
    </View>
  );
}
```

Pan to scroll, pinch to zoom, and drag the price/time axes to rescale them.

### Props

| Prop               | Type                          | Description                                                                                                          |
| ------------------ | ----------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `candles`          | `Candle[]`                    | Required. OHLCV data (`timeMs`, `open`, `high`, `low`, `close`, `volume`).                                           |
| `style`            | `StyleProp<ViewStyle>`        | Root view style. Defaults to filling the parent.                                                                     |
| `width`            | `number`                      | Explicit width override in px. Default: measured from the parent.                                                    |
| `height`           | `number`                      | Explicit height override in px. Default: measured from the parent.                                                   |
| `visibleRange`     | `{ startMs, endMs }`          | Time window to render. Omit to fit all candles.                                                                      |
| `theme`            | `VroomTheme`                  | Color overrides (`background`, `bull`, `bear`, `borderBull`, `borderBear`, `wickBull`, `wickBear`, `accentBull`, `accentBear`, `grid`, `axisText`, `crosshair`, `crosshairTarget`). |
| `onCrosshair`      | `(e: CrosshairEvent) => void` | Fires as the crosshair moves (`active`, `timeMs`, `price`).                                                          |
| `onViewportChange` | `(startMs, endMs) => void`    | Fires when the visible time range changes via gesture.                                                               |

### iOS & Android (coming soon)

Standalone Swift and Kotlin packages with idiomatic native APIs are planned. For now, all
platforms are consumed through the React Native package above.

## Layout

- `packages/core/` — platform-agnostic C++ engine (Skia-based). Public C facade.
- `packages/react-native/` — RN npm package. JSI bridge + iOS/Android shims around the core.
- `examples/test-bench/` — Expo dev-client test bench app.
- `docs/` — Docusaurus documentation site.
- `third_party/skia/` — fetched prebuilts (gitignored).

Future:

- `packages/ios/` — standalone SwiftPM/CocoaPod with idiomatic Swift API.
- `packages/android/` — standalone AAR with idiomatic Kotlin API.

## Dev quickstart

```sh
pnpm install
pnpm example          # runs the RN test bench
```

## Docs

Full documentation is a [Docusaurus](https://docusaurus.io) site in
[`docs/`](docs/), deployed as a static site (Vercel). Content lives in
`docs/content/`; the API reference under `docs/content/reference/` is generated
from the React Native package's types by `docusaurus-plugin-typedoc` on every
build, so it can't drift from the source.

```sh
pnpm docs          # local dev server
pnpm docs:build    # production build (regenerates the API reference)
```

After changing public types, run `pnpm docs:build` and commit the regenerated
`docs/content/reference/` (CI verifies it's fresh).

## Architecture

Short version: `packages/core/` exposes a thin `extern "C"` API
(`vroom_chart.h`). Each distribution wraps that facade — RN via JSI, iOS via Swift, Android
via JNI. The core owns all Skia drawing; consumers only provide an `SkCanvas` and forward
gesture events.
