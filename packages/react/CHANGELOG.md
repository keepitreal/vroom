# @vroomchart/react

## 0.3.0

### Minor Changes

- c09c30f: Line tool editing + managed drawing persistence (web):

  - **Select, edit, and delete lines** — click a line to select it (endpoint handles
    appear), drag a handle to move an endpoint, drag the line body to translate the
    whole line, and press Delete/Backspace to remove a selected line.
  - **Copy / paste** a selected line with Cmd/Ctrl+C / Cmd/Ctrl+V — pasted under the
    crosshair at the same grab point, or above/below the original when the pointer
    hasn't moved.
  - **Shift-constrained drawing** — hold Shift while placing the second point to snap
    the line to the nearest 45°.
  - **Cancel an in-progress line** — Escape/Delete/Backspace after the first point
    cancels it while staying in draw mode.
  - **Managed drawing persistence** — new optional `drawingStore` prop. Provide a
    small string key-value adapter and the chart owns, loads, and saves drawings for
    you (keyed by `seriesKey`, so they persist across timeframes but not markets),
    instead of wiring the controlled `drawings` prop + `onDrawing*` callbacks. The
    stored string is a versioned, migratable envelope owned by the library, so the
    adapter never changes as the drawing schema grows. `serializeDrawings` /
    `deserializeDrawings` helpers are exported for pre-seeding or import/export.

  All additive — the controlled `drawings` model is unchanged. Drawing tools remain
  web-only.

### Patch Changes

- Updated dependencies [c09c30f]
  - @vroomchart/core-wasm@0.3.0

## 0.2.0

### Minor Changes

- Candle styling + initial-zoom controls (web + React Native):

  - `defaultCandleWidth` prop — drive the initial zoom from a target candle body width.
  - Candle body border now draws **inside** the body (never changes candle width) and can be hidden.
  - New `setFloat` theme-float bridge, exposing configurable numeric/boolean theme fields:
    - `theme.wickWidth` — wick stroke width (px)
    - `theme.candleRadius` — candle body corner radius (px)
    - `theme.wickRoundCap` — rounded wick end caps
    - `theme.volumeRadius` — rounded top corners on volume bars

  All new styling defaults to off, so existing charts are visually unchanged.

### Patch Changes

- Updated dependencies
  - @vroomchart/core-wasm@0.2.0

## 0.1.7

### Patch Changes

- cb61128: feat: liquidity bands overlay (order-book depth)

  Render resting-order / L2 liquidity as horizontal price-interval bands behind the
  candles. Each band is anchored at the inner edge of the price axis, stretches left
  and fades out with a gradient, is colored by buy/sell side, and takes its opacity
  from the order volume. Bands are defined in price space so they scale with the
  y-axis. New controlled `liquidity` prop on `<VroomChart>` (`LiquidityConfig` /
  `LiquidityBand`), plumbed through the core (`vroom_chart_set_liquidity`), the WASM
  handle (`setLiquidity`), and React.

- Updated dependencies [cb61128]
  - @vroomchart/core-wasm@0.1.5

## 0.1.6

### Patch Changes

- fd4636d: Fix cross-chart viewport reset with gappy candle series. Streaming an in-place update (or append) to a chart whose series has interior gaps — missing bars from downtime / illiquid periods — was misclassified as a full data reset, snapping the panned/zoomed viewport back to auto-fit. Stream detection now locates the previous last bar by timestamp (binary search) rather than a uniform-grid index, so it tolerates non-uniform grids. Most visible with two charts on different timeframes, where the deeper-history pane accumulates gaps.

## 0.1.5

### Patch Changes

- c9c78da: Cross-chart crosshair sync: hovering one chart can drive another's crosshair. Adds `price` to the `onCrosshair` event, a controlled `crosshairOverride` prop to mirror a crosshair in data space, and the `setCrosshairData(timeMs, price)` handle / `vroom_chart_set_crosshair_data` core method. The `onCrosshair` `move` event now also fires on vertical (price) moves within a candle so the synced price stays current.
- Updated dependencies [c9c78da]
  - @vroomchart/core-wasm@0.1.4

## 0.1.4

### Patch Changes

- Republish: 0.1.3 shipped an unresolvable `workspace:*` dependency on @vroomchart/core-wasm (published with npm publish, which does not rewrite the workspace protocol)

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
