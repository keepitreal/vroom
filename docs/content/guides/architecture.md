# Architecture

vroom's architecture is designed around a single principle: **one rendering core, many platforms**. A shared C++ engine built on Skia handles all drawing, while thin platform-specific wrappers handle the canvas surface and input events.

## High-level design

```
┌─────────────────────────────────────────────┐
│                  Your App                    │
├──────────────┬──────────────┬────────────────┤
│  React Native│   React DOM  │  (future)      │
│  VroomChart  │  VroomChart  │  Swift / Kotlin│
├──────────────┼──────────────┼────────────────┤
│  JSI Bridge  │  WASM glue   │  FFI           │
├──────────────┴──────────────┴────────────────┤
│              C++ Core (Skia)                 │
│  ┌──────────────────────────────────────┐    │
│  │  extern "C" facade (vroom_chart.h)   │    │
│  ├──────────────────────────────────────┤    │
│  │  Renderer  │  Layout  │  Indicators  │    │
│  ├──────────────────────────────────────┤    │
│  │  Gestures  │  Drawings│  Liquidity   │    │
│  └──────────────────────────────────────┘    │
└──────────────────────────────────────────────┘
```

## The C++ core

The core (`packages/core/`) is a C++17 library that does all the heavy lifting:

- **Rendering**: Draws candles, volume bars, indicators, gridlines, axes, crosshair, and drawing tools directly to an `SkCanvas`
- **Layout**: Computes pixel positions from data-space coordinates, handling pan/zoom transforms
- **Indicators**: Computes RSI, MACD, VWAP, SMA, and EMA from OHLCV data
- **Gestures**: Processes pan, pinch, zoom, and axis-drag inputs, updating the viewport
- **Crosshair**: Snaps to candle-aligned time slots and computes OHLCV at the cursor position

The core exposes a public `extern "C"` facade in `vroom_chart.h`, making it callable from any language with C FFI support (JSI, WASM, JNI, Swift).

## Platform wrappers

### React Native (`react-native-vroom-chart`)

```
React Native JS
  └── JSI (C++ bridge)
        └── vroom C++ core (statically linked)
              └── Skia canvas from @shopify/react-native-skia
```

- The JSI bridge calls the C++ core directly — no serialization overhead
- Gesture handlers (react-native-gesture-handler + Reanimated worklets) forward events to the core
- The core renders to a Skia canvas provided by `@shopify/react-native-skia` ≥ 2.11 (which also needs `react-native-worklets` ≥ 0.7)

### Web (`@vroomchart/react` + `@vroomchart/core-wasm`)

```
React DOM JS
  └── @vroomchart/react
        └── @vroomchart/core-wasm (JS loader)
              └── vroom_core.wasm (Emscripten-compiled C++ core)
                    └── Skia-WASM canvas
```

- The C++ core is compiled to WebAssembly via Emscripten
- `@vroomchart/core-wasm` loads the WASM module and provides `loadVroom()`
- `@vroomchart/react` wraps it in a React component with Pointer/Wheel event forwarding
- WASM assets (`.wasm`, `.mjs`, `.ttf`) are bundled inside the package — consumers don't host them

## Data flow

```
App provides Candle[] + props
  ↓
Platform wrapper forwards to C++ core
  ↓
Core computes layout + renders to Skia canvas
  ↓
Canvas pixels displayed on screen
  ↓
User gesture → platform wrapper → core updates viewport → re-render
```

Events flow back up:
```
Core fires event (crosshair, viewport change)
  ↓
Platform wrapper calls your callback (onCrosshair, onViewportChange)
  ↓
Your app reacts (render OHLCV readout, haptics, fetch more data)
```

## Future platforms

The `extern "C"` facade means the same core can be consumed by:

- **Android (native)**: JNI bridge → AAR with idiomatic Kotlin API
- **iOS (native)**: Swift FFI → SwiftPM with idiomatic Swift API
- **Desktop**: Native windowing (Skia's own backend)

Each platform wrapper would own the canvas surface and gesture forwarding, identical to the existing React Native and web patterns.
