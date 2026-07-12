# @vroomchart/core-wasm

## 0.1.5

### Patch Changes

- cb61128: feat: liquidity bands overlay (order-book depth)

  Render resting-order / L2 liquidity as horizontal price-interval bands behind the
  candles. Each band is anchored at the inner edge of the price axis, stretches left
  and fades out with a gradient, is colored by buy/sell side, and takes its opacity
  from the order volume. Bands are defined in price space so they scale with the
  y-axis. New controlled `liquidity` prop on `<VroomChart>` (`LiquidityConfig` /
  `LiquidityBand`), plumbed through the core (`vroom_chart_set_liquidity`), the WASM
  handle (`setLiquidity`), and React.

## 0.1.4

### Patch Changes

- c9c78da: Cross-chart crosshair sync: hovering one chart can drive another's crosshair. Adds `price` to the `onCrosshair` event, a controlled `crosshairOverride` prop to mirror a crosshair in data space, and the `setCrosshairData(timeMs, price)` handle / `vroom_chart_set_crosshair_data` core method. The `onCrosshair` `move` event now also fires on vertical (price) moves within a candle so the synced price stays current.

## 0.1.3

### Patch Changes

- 955df2c: Improve chart handling when switching time frames and assets: constant candle width across timeframe changes, full view reset on symbol change, continuous y auto-fit

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

## 0.1.1

### Patch Changes

- x-axis: calendar-aware tick labels (month/year cadences) so labels no longer overlap when zoomed out, and extend the future-scroll cap to 25% candles visible with rubber-band resistance.

## 0.1.0

### Minor Changes

- 5649f45: Initial public release.

  - `react-native-vroom-chart` — Skia candlestick chart for React Native (iOS native + JS), with RSI/MACD/MA/VWAP indicators, theming, crosshair, and pan/zoom/axis gestures.
  - `@vroomchart/core-wasm` — framework-agnostic web core: the C++/Skia renderer compiled to WebAssembly (bundled), with a Canvas2D fallback.
  - `@vroomchart/react` — React DOM `<VroomChart>` over the WASM core, API-matched to the React Native component.
