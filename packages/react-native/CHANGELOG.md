# react-native-vroom-chart

## 0.1.1

### Patch Changes

- x-axis: calendar-aware tick labels (month/year cadences) so labels no longer overlap when zoomed out, and extend the future-scroll cap to 25% candles visible with rubber-band resistance.

## 0.1.0

### Minor Changes

- 5649f45: Initial public release.

  - `react-native-vroom-chart` — Skia candlestick chart for React Native (iOS native + JS), with RSI/MACD/MA/VWAP indicators, theming, crosshair, and pan/zoom/axis gestures.
  - `@vroomchart/core-wasm` — framework-agnostic web core: the C++/Skia renderer compiled to WebAssembly (bundled), with a Canvas2D fallback.
  - `@vroomchart/react` — React DOM `<VroomChart>` over the WASM core, API-matched to the React Native component.
