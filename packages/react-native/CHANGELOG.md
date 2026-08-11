# react-native-vroom-chart

## 0.6.0

### Minor Changes

- b256f19: Implement the Android native bridge. `VroomChart` now renders and responds to
  gestures (pan, pinch, crosshair, axis scaling) on Android via a JNI/JSI bridge
  into the same C++ chart core and RN-Skia integration the iOS bridge uses.
  Previously the Android TurboModule was a placeholder and `VroomChart` was
  non-functional on Android.
- eead2e2: Add consumer-supplied horizontal price status lines, in the style of the order
  and position lines trading terminals draw. Pass `priceLines` to render a line at
  a price with a label group — body text, an optional quantity segment, and an
  optional close button — plus a matching badge in the price axis.

  Lines can be dragged vertically when `draggable` is set: `onPriceLineDrag` fires
  continuously and `onPriceLineDragEnd` fires on drop, and because `priceLines` is
  a controlled prop a host that rejects the move simply doesn't update state and
  the line snaps back. Supplying `onPriceLineClose` renders the close button on
  lines marked `closable`. Escape cancels an in-progress drag on web, where the
  hovered segment also brightens and the cursor becomes `ns-resize`.

  Rendering and hit-testing live in the C++ core, so web and React Native share
  one implementation.

### Patch Changes

- 9eeb60a: Fix axis labels (price and time) never rendering. Two bugs compounded: on
  Android, obtaining RN-Skia's platform context used a `dynamic_pointer_cast`
  that always fails across the `librnskia.so`/`libvroomchart.so` boundary (same
  class of cross-`.so` RTTI mismatch as the `JsiSkPicture` bridge), so the axis
  typeface was never loaded at all; separately, on both platforms, requesting
  the "default" system font with a null family name isn't honored by Android's
  font manager (`SkFontMgr_New_Android`), which now falls back to `sans-serif`.

## 0.5.0

### Minor Changes

- 9feb921: Add a Bollinger Bands overlay indicator (`bollingerBands` prop): a basis
  SMA/EMA over a configurable period with upper/lower bands at ± N population
  standard deviations, per-line colors/widths, and an optional translucent fill
  between the bands. Defaults match the standard settings (20-period SMA of close, ±2σ).

## 0.4.0

### Minor Changes

- c2f1d1a: New **box** drawing tool (web) — an axis-aligned rectangle annotation, alongside the
  existing line:

  - **`tool="box"`** — in `draw` mode, click to place one corner and click again to
    commit. A live preview rectangle tracks the cursor between the two clicks; hold
    **Shift** to constrain it to a perfect square.
  - **4 corner anchors** — selecting a box shows a handle on each corner. Dragging one
    resizes the box from that corner while the diagonally opposite corner stays fixed,
    so all four corners always stay at 90°. Shift snaps to a square here too.
  - **Move / edit** — the faint interior fill is grabbable to drag the whole box; delete
    and copy/paste work exactly as they do for lines.
  - **Persistence** — boxes serialize through the same `drawingStore` envelope as lines.
    The `Drawing` type's `type` field widens to `'line' | 'box'`, with `points` holding
    the two opposite corners; `DrawTool` widens to `'line' | 'box'`.

  Additive — existing line drawings and their behavior are unchanged.

- b705ea8: New **pencil** drawing tool (web) — freehand strokes, alongside the existing line
  and box:

  - **`tool="pencil"`** — press and drag to draw for as long as the button is held;
    releasing commits the stroke (`onDrawingComplete`). The tool stays active so you
    can draw several strokes in a row. The path is rendered as a smoothed,
    constant-width stroke with round caps.
  - **Translate-only editing** — a committed stroke is never reshaped. Selecting it
    shows anchors on its first and last point as a visual cue that it can be moved;
    dragging an anchor translates the whole path exactly like dragging any other
    part of it. Delete and copy/paste work as they do for the other tools.
  - **Automatic thinning** — a drag generates hundreds of samples, so strokes are
    simplified before being committed and persisted (typically a few dozen points),
    keeping `drawingStore` payloads reasonable.

  **`Drawing` is now a discriminated union.** `'line'` and `'box'` keep their exact
  `[DrawPoint, DrawPoint]` tuple; `'pencil'` carries a variable-length
  `DrawPoint[]`. Code that reads `points[1]` on a general `Drawing` now needs to
  narrow on `type` first — existing code that already works with a known line or
  box is unaffected. `LineDrawing`, `BoxDrawing` and `PencilDrawing` are exported.

  Persisted payloads are unchanged in shape (no envelope version bump); a build
  without the pencil tool drops unknown strokes on load as before. Deserialization
  now also validates each drawing's `points`, so a corrupt entry from a store is
  dropped instead of reaching the renderer.

## 0.3.0

### Minor Changes

- 727c79d: Line chart mode with an animated candle↔line transition (web + React Native):

  - **`chartType` prop** (`'candles' | 'line'`) — `'line'` hides the candles and draws
    a polyline through each candle's close. Volume, indicators (RSI/MACD/MAs/VWAP),
    crosshair, and drawings all keep rendering. Style the line with `theme.lineColor`
    and `theme.lineWidth`.
  - **Animated transition** — switching `chartType` smoothly morphs between the two:
    each candle collapses toward its close as the connecting line fades in (and the
    reverse). Control the duration with the new **`transitionMs`** prop (default ~300 ms;
    `0` snaps instantly). Honors `prefers-reduced-motion` on web (a plain cross-fade with
    no vertical motion) and retargets smoothly if toggled mid-transition.

  Both are additive and default to the previous behavior (candlesticks).

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
