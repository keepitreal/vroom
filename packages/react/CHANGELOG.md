# @vroomchart/react

## 0.10.0

### Minor Changes

- b053080: **The line chart's corners can now be rounded** with a new `lineTension` theme
  field, from `0` (the existing straight segments between closes) to `1` (fully
  smooth). Defaults to `0`, so nothing changes without opting in:

  ```tsx
  theme={{ lineTension: 0.5 }}
  ```

  The curve is monotone-limited rather than a plain spline, which matters for
  price: smoothing can never overshoot into a price that didn't trade. Every peak
  and trough stays on an actual close, and the curve never leaves the range of the
  two closes it connects — so a rounded line can't invent a high that would read as
  a level someone might trade against. The gradient fill beneath the line follows
  the same path, so the two stay flush.

- b053080: **The line chart's newest end is now marked with a dot**, with an optional pulse
  expanding out from behind it. Two new boolean theme fields:

  ```tsx
  theme={{ lineTipDot: true, lineTipPulse: true }}
  ```

  `lineTipDot` defaults to **true**, so line-mode charts pick up the dot without
  any change — the one visible difference in this release for existing consumers.
  Set it to `false` to opt out. Everything about it is derived rather than
  separately configurable: the dot takes `line` for its color and scales its radius
  off `lineWidth`, wrapped in a 2px ring of `background` that separates it from the
  line's own stroke cap.

  `lineTipPulse` defaults to **false**. When on, a ring expands out of the dot and
  fades, once every 2.6 seconds, with a rest between beats — modeled on
  TradingView's last-price animation. Because the ring never settles, turning it on
  keeps the chart repainting continuously; leave it off for charts that should be
  able to go idle. An OS reduced-motion preference suppresses the pulse (the dot
  stays), the same bargain the candle↔line morph already strikes when it keeps its
  crossfade but skips the collapse.

  The pulse is driven entirely by the core's frame clock rather than a host
  animation loop, so it needs nothing from consumers and stays in step with the
  candle↔line and interval morphs — the dot rides the line's end through a
  timeframe switch instead of detaching from it.

  **The default view now leaves a gap between the newest candle and the price
  axis**, on candles and the line alike. Previously the newest bar sat flush
  against the axis, which read as cramped and left the pulse ring nowhere to expand
  into. Charts that pass an explicit `visibleRange` are unaffected, and a timeframe
  switch carries the gap over so it stays put.

### Patch Changes

- Updated dependencies [b053080]
- Updated dependencies [b053080]
  - @vroomchart/core-wasm@0.10.0

## 0.9.0

### Minor Changes

- 5e26557: **New `path` drawing tool** — a multi-segment polyline that ends in an arrowhead,
  for marking out a move leg by leg.

  Set `tool="path"` in draw mode. The first click drops the starting vertex and
  each click after adds a segment, with a preview segment tracking the cursor in
  between. Since there's no point count to finish on, the path ends when you say
  so: **Escape**, **double-click**, or **right-click** all keep it as drawn, and
  **⌘Z / Backspace / Delete** take back the last vertex. Hold **Shift** to
  constrain a segment to 45°. Once committed every vertex is its own drag handle,
  so a leg can be reshaped without redrawing. Capped at 64 vertices.

  `DrawTool` gains `'path'` and the `Drawing` union gains a `PathDrawing` variant
  (variable-length `points`, like `'pencil'`). Code that already narrows on `type`
  before reading `points[1]` is unaffected. `LineDrawing`, `BoxDrawing`,
  `PencilDrawing` and `PathDrawing` are now all exported from `@vroomchart/react`.

  Persisted payloads are unchanged in shape (no envelope version bump); a build
  without the path tool drops unknown drawings on load as before.

  Drawing remains web-only, so `react-native-vroom-chart` is unaffected.

### Patch Changes

- Updated dependencies [5e26557]
  - @vroomchart/core-wasm@0.9.0

## 0.8.0

### Minor Changes

- a5df8f1: Animate the candles themselves across a timeframe switch: each one slides and
  stretches into the shape of its counterpart in the new interval instead of the
  whole series jumping.

  Because the time axis preserves each candle's pixel slot across a switch, the
  k-th candle from the right edge occupies the same columns before and after — so
  only y has to move. The core now captures the outgoing wick and body geometry
  (as price-band fractions, which keeps frame 0 pixel-identical to the pre-switch
  frame and survives a resize mid-animation) and interpolates each slot toward its
  new counterpart. A candle that changes direction crossfades its color; a slot
  present on only one side fades rather than pops.

  Add `transitionEasing` (`'linear' | 'ease-in' | 'ease-out' | 'ease-in-out'`,
  default `'ease-in-out'`) alongside `transitionMs`, which now covers both this
  animation and the candle↔line switch. `transitionMs={0}` or an OS reduced-motion
  preference snaps as before, skipping the capture entirely.

  Two new handle methods back this, both web-only for now:
  `beginIntervalMorph` captures the geometry and `setIntervalMorph` advances the
  animation. Volume bars, overlays and indicators still snap.

- 6ece5a9: Give every indicator config one set of naming conventions, and bring RSI and
  VWAP up to the styling other indicators already had.

  RSI now takes colors, widths, and visibility for the index line, the trendline,
  and the pair of dashed band rules, plus an SMA/EMA choice for the trendline
  (`maType`). VWAP keeps its `color` and `width` but is now marshalled as a struct
  like the rest. Both are unchanged visually until you set something: unset colors
  and widths still resolve to the stock look in the renderer.

  `'sma' | 'ema'` is now the shared `MAKind` union wherever it appears, matching
  how `MASource` is already shared.

  Renamed fields:

  | Before                                               | After                                     |
  | ---------------------------------------------------- | ----------------------------------------- |
  | `MovingAverageOverlay.kind`                          | `maType`                                  |
  | `MovingAverageOverlay.length`                        | `period`                                  |
  | `BollingerBandsConfig.basis`                         | `maType`                                  |
  | `BollingerBandsConfig.fill`                          | `fillVisible`                             |
  | `RSIConfig.maEnabled`                                | `maVisible`                               |
  | `MACDConfig.macdColor` / `macdWidth` / `macdVisible` | `lineColor` / `lineWidth` / `lineVisible` |

  The low-level `VroomChartHandle.setRSI` and `setVWAP` now take a single spec
  object instead of positional arguments, matching `setMACD` and `setBollinger`.

- a5df8f1: Line-chart mode now defaults to a violet stroke and fills a gradient beneath the
  line.

  The default `lineColor` moves from the neutral foreground to `#8957e5`, matching
  the RSI line. A gradient using that same color is now filled under the polyline,
  strongest at the line's peak and ramping to fully transparent at the bottom of
  the price pane. It fades in with the candle-to-line morph and reshapes with the
  line through a timeframe switch, since it's built from the same vertices.

  New `theme.lineGradientOpacity` sets the fill's strength at its strongest point
  (default 0.28); set it to 0 to render the bare line as before.

  The fill draws behind the volume bars rather than with the line, so bars stay
  legible on top of it. Skia's linear-gradient API differs between the Skia the
  WASM build uses and the one react-native-skia bundles; the selection shim that
  already existed for liquidity bands moved into a shared `gradient.h` so both
  layers build the ramp the same way.

- a5df8f1: Animate the line chart across a timeframe switch: each vertex slides to its new
  close instead of the whole polyline jumping.

  Line mode reuses the geometry the candle interval morph already captures — a
  snapshot's `x` and `close` fractions are exactly what a polyline vertex needs —
  so `transitionMs` and `transitionEasing` drive it with no extra API. Vertices
  pair by slot, the same position-from-the-right-edge rule the candle morph uses,
  so only y moves.

  The close line now has its own entry point rather than borrowing the MA-overlay
  routine, which also drops a per-frame heap allocation on the line-mode draw path.
  Indicator overlays (SMA/EMA, VWAP, Bollinger) and volume still snap; their series
  aren't candle closes, so the capture can't stand in for them.

- 6ece5a9: Expand the MACD config beyond its four lengths.

  Inputs gain a price `source` and an SMA/EMA choice for the fast and slow legs
  (`maType`) and for the signal line (`signalMaType`). Styling gains per-series
  color, width, and visibility for the MACD and signal lines, four histogram
  colors (above and below zero, each with a shade for bars building and a lighter
  one for bars easing back toward zero), and a configurable zero line. Hiding a
  series rescales the pane around what is left on show.

  Every new field is optional and falls back to the previous look, so existing
  charts render unchanged: histogram bars still follow `theme.accentBull` /
  `theme.accentBear`, and an unset fading color derives from its base at half
  opacity.

  The low-level `VroomChartHandle.setMACD` now takes a single spec object instead
  of four positional arguments. Consumers of the `<VroomChart>` components are
  unaffected.

- a5df8f1: Scale-lock the price axis across a timeframe switch, so candles keep their
  vertical size when the interval changes.

  Switching intervals re-buckets the same price action into a smaller or larger
  high-low span. The time axis already preserved each candle's pixel width; the
  price axis dropped back to auto-fit, which threw away any zoom the user had
  applied to the y-axis and made the candles abruptly shrink or grow. The switch
  now rescales that zoom instead: the visible high-low envelope keeps the exact
  pixel height and position it had a frame earlier, and the price labels crossfade
  to the new values.

  Auto-y is unchanged — auto-fit widens the envelope by a fixed factor, so its
  pixel height was already interval-invariant.

  Two new handle methods back this, both web-only for now: `getVisiblePriceEnvelope`
  reads the visible min-low/max-high, and `preservePriceEnvelope` applies the
  rescale (a no-op in auto-y mode).

- a5df8f1: Volume bars are now configurable through a `volume` prop, alongside the other
  indicators:

  ```tsx
  <VroomChart
    candles={candles}
    volume={{ enabled: true, opacity: 0.35, height: 0.25, radius: 2 }}
  />
  ```

  - `enabled` toggles the bars. It defaults to `true`, so omitting the prop leaves
    existing charts unchanged; pass `{ enabled: false }` to hide them.
  - `opacity` (default 0.5) sets how much quieter the bars read than the candles.
  - `height` (default 0.2) is the tallest bar as a fraction of the price pane.
    It's a ceiling rather than a reserved strip, matching the conventional volume
    overlay: raising it lets the bars reach further up over the candles instead of
    compressing them.
  - `radius`, `upColor` and `downColor` restyle the bars, defaulting to
    `theme.volumeRadius` and `theme.accentBull` / `theme.accentBear`.

  `theme.volumeRadius` is deprecated in favor of `volume.radius`, which sits with
  the rest of the volume styling. It still applies when `volume.radius` is omitted.

- a5df8f1: Toggling `volume.enabled` now animates the bars instead of snapping them.

  Turning volume off sinks the bars out of view in a staggered cascade: each bar
  falls over the last `height / tallestHeight` of the transition, so the tallest
  starts immediately and takes the whole window, shorter bars start progressively
  later, and every bar reaches the axis at the same moment. Turning volume back on
  plays the same timeline backwards, which brings the shortest bars home first and
  lands the tallest last.

  Because each bar runs on its own window, the easing has to be applied per bar
  rather than to the animation as a whole — the host feeds the core linear progress
  and the core applies `transitionEasing` inside each window. A bar too short to
  warrant its own window gets a floor so it falls rather than pops, and toggling
  mid-animation reverses from wherever the bars are instead of jumping.

  `transitionMs` and `transitionEasing` drive it, matching the candle↔line and
  timeframe transitions; `transitionMs: 0` or `prefers-reduced-motion` still snaps.
  No API change — `volume.enabled` is unchanged.

### Patch Changes

- a5df8f1: Fix axis label fades never animating on web, and swap the axes with a fade-out
  then fade-in across a timeframe switch instead of letting them snap.

  The label fade machinery has been in place for a while, but on web it never ran:
  `vroom_chart_draw` — the entry point the WASM build uses — called `draw_chart`
  directly, while the per-frame `dt` was computed in `rebuild_chart_picture`, which
  only the React Native SkPicture path goes through. `dt` therefore sat at 0
  forever, and 0 is the fade updaters' "snap to target" signal. The clock now lives
  in `draw_chart`, so both hosts tick it.

  A second snap hid behind the first: a gap over 100ms clamped `dt` to 0, the very
  thing the surrounding comment said the clamp was there to prevent. Since a
  discrete event (timeframe switch, asset switch) always starts from an idle chart,
  its first frame took the snap path and finished every fade at once. Long gaps now
  clamp to a nominal frame instead.

  An interval morph then hands the axes to a two-phase envelope: the pre-switch
  ticks fade out over the first half of the morph, the new ones fade in over the
  second. Nothing translates. Both halves are clocked off the morph's eased
  progress, so the axes take `transitionMs`, follow `transitionEasing`, and land
  with the candles. While fading out, the axes stay laid out against the captured
  pre-switch scale and window rather than the new ones, so a label is never drawn
  anywhere it wasn't already; the tick set is swapped at the midpoint, when the axis
  is fully transparent.

- 6ece5a9: Show the newest candle on first render instead of hiding it behind the price
  axis.

  The default framing pinned the right edge of the visible window to the last
  candle's `timeMs`. A candle occupies the slot `[timeMs, timeMs + duration)` and
  draws centered in it, so that put the newest candle's center half a slot past
  the plot area and its whole body inside the reserved y-axis strip, which the
  renderer then paints over. The last-price line and badge still rendered, so the
  price was visible but the candle it came from was not — it only appeared once
  the user panned right.

  The window now ends at the last slot's end, so the newest candle sits flush
  against the right edge of the plot area. This applies to both the
  `defaultCandleWidth`-driven framing and the legacy "most recent ~80 candles"
  fallback, and to `resetView` since it re-runs the same framing.

- Updated dependencies [a5df8f1]
- Updated dependencies [a5df8f1]
- Updated dependencies [6ece5a9]
- Updated dependencies [6ece5a9]
- Updated dependencies [a5df8f1]
- Updated dependencies [a5df8f1]
- Updated dependencies [6ece5a9]
- Updated dependencies [a5df8f1]
- Updated dependencies [a5df8f1]
- Updated dependencies [a5df8f1]
  - @vroomchart/core-wasm@0.8.0

## 0.7.0

### Minor Changes

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

- Updated dependencies [eead2e2]
  - @vroomchart/core-wasm@0.7.0

## 0.6.0

### Minor Changes

- 9feb921: Add a Bollinger Bands overlay indicator (`bollingerBands` prop): a basis
  SMA/EMA over a configurable period with upper/lower bands at ± N population
  standard deviations, per-line colors/widths, and an optional translucent fill
  between the bands. Defaults match the standard settings (20-period SMA of close, ±2σ).

### Patch Changes

- Updated dependencies [9feb921]
  - @vroomchart/core-wasm@0.6.0

## 0.5.0

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

### Patch Changes

- Updated dependencies [c2f1d1a]
- Updated dependencies [a947d48]
- Updated dependencies [b705ea8]
  - @vroomchart/core-wasm@0.5.0

## 0.4.0

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

### Patch Changes

- Updated dependencies [727c79d]
  - @vroomchart/core-wasm@0.4.0

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
