# @vroomchart/react

## 0.1.3

### Patch Changes

- 955df2c: Improve chart handling when switching time frames and assets: constant candle width across timeframe changes, full view reset on symbol change, continuous y auto-fit
- Updated dependencies [955df2c]
  - @vroomchart/core-wasm@0.1.3

## 0.1.2

### Patch Changes

- Customizable candle/indicator styling and new web gestures.

  - Per-variant candle border and wick colors (`borderBull`/`borderBear`,
    `wickBull`/`wickBear`) that inherit the body fill when left transparent.
  - Generic `accentBull`/`accentBear` colors for the current-price indicator,
    volume bars, and MACD histogram, decoupled from the candle bull/bear colors.
  - Web: draggable separator to resize the indicator band vs the price pane
    without rescaling candles, and a scalable indicator y-axis (drag the strip
    beside an RSI/MACD pane to zoom its value range independently).

- Updated dependencies
  - @vroomchart/core-wasm@0.1.2

## 0.1.1

### Patch Changes

- x-axis: calendar-aware tick labels (month/year cadences) so labels no longer overlap when zoomed out, and extend the future-scroll cap to 25% candles visible with rubber-band resistance.
- Updated dependencies
  - @vroomchart/core-wasm@0.1.1

## 0.1.0

### Minor Changes

- 5649f45: Initial public release.

  - `react-native-vroom-chart` — Skia candlestick chart for React Native (iOS native + JS), with RSI/MACD/MA/VWAP indicators, theming, crosshair, and pan/zoom/axis gestures.
  - `@vroomchart/core-wasm` — framework-agnostic web core: the C++/Skia renderer compiled to WebAssembly (bundled), with a Canvas2D fallback.
  - `@vroomchart/react` — React DOM `<VroomChart>` over the WASM core, API-matched to the React Native component.

### Patch Changes

- Updated dependencies [5649f45]
  - @vroomchart/core-wasm@0.1.0
