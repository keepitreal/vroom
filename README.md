# vroom

Mobile-first Skia candlestick chart library. A single C++ core shipped to iOS, Android, and React Native.

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
