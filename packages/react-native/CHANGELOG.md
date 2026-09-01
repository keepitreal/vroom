# react-native-vroom-chart

## 0.13.1

### Patch Changes

- 56948f1: Align `libvroomchart.so` to 16KB ELF pages for Android 15+.

  NDK r27 and older default to 4KB LOAD alignment (`align=2**12`). Apps targeting
  SDK 35 then fail Play's 16KB page-size check for both arm64-v8a and x86_64.
  The Android CMake target now passes `-Wl,-z,max-page-size=16384` (and
  `common-page-size`) so consumers no longer need to patch the published
  package. NDK r28+ already aligns to 16KB; the flags are redundant there.

- ca60911: Stop Android pan/zoom from OOM-killing the app.

  Every gesture frame serialized the chart `SkPicture` into RN-Skia, and the
  default serializer embedded the system typeface (megabytes) each time. RSS
  climbed to multiple GB in seconds and the process vanished with no JS error.
  Android now rasterizes to an `SkImage` (one framebuffer) and only falls back
  to serialize without embedding font bytes.

## 0.13.0

### Minor Changes

- 8a7f04d: Abbreviate million/billion/trillion y-axis labels as M / B / T.

  A market-cap series in the tens of millions was printing `32,192,381.38` and
  widening the price strip to hold it. Assets at or above a million now share a
  compact format (`32.19M`, `34.54B`, `1.20T`) across ticks, the last-price pill,
  the crosshair, and price-line badges. The axis strip sizes to the shorter
  strings. Prices below a million, including thousands with grouping, are
  unchanged.

## 0.12.0

### Minor Changes

- 63068c2: Add `theme.badgeText`, controlling the text drawn on filled badges.

  The current-price indicator's label was hardcoded white, along with the
  crosshair's price and time badges and the price-line pills. All four now read
  one theme color, so a light theme can make them legible instead of leaving
  white text on a pale fill.

  ```tsx
  <VroomChart theme={{ badgeText: "#1b1f24" }} />
  ```

  Defaults to white, so existing themes render exactly as before.

  **Breaking (core-wasm only):** the `ColorKey` enum drops `TooltipBg` and
  `TooltipText`. Both were vestigial — they had palette defaults but were never
  read by any draw call and were not reachable through `VroomTheme`. Slot 8 is
  now `ColorKey.BadgeText`; slot 7 is `Reserved7`, held empty so the indices
  after it don't shift, since the theme maps hardcode them.

- 63068c2: Add `theme.showXAxis` / `theme.showYAxis` to hide either axis.

  Hiding an axis collapses its strip and hands the reclaimed space to the plot, so
  the candles grow to fill it. The strip's contents — labels, the current-price
  badge, the crosshair and price-line badges — fade out over the first half of the
  collapse, so text is never squeezed into a strip too narrow to hold it. Both
  default to `true` and animate over `transitionMs`, snapping under reduced motion.

  Also fixes the volume-collapse animation not handing back to the render loop
  when it finishes, which could leave the line-tip pulse frozen after toggling
  volume — the same defect fixed for the candle↔line morph in #50.

### Patch Changes

- 3494fa2: Fix the line-tip pulse freezing after a candle → line switch.

  With `lineTipPulse` on, the ring stopped animating once you switched to candle
  mode and back. The chart drives two independent frame loops: the candle↔line
  morph runs its own clock, and a second loop sustains anything the core reports
  as still animating. Settling in line mode is the moment the pulse turns on, but
  the morph loop ended without handing frames over, so the ring sat frozen until
  an unrelated repaint — a pan, a zoom, a data push — happened to restart the
  other loop.

  Every exit from the morph now hands off: the animated completion, the
  `transitionMs: 0` snap, the fresh-handle snap, and the already-at-target
  no-op. Settling in candle mode is unaffected, since there is nothing to
  animate. Reduced motion is also unaffected, as it disables the pulse outright.

## 0.11.1

### Patch Changes

- 92f4c1e: Expose the selected drawing, add `fill` and `locked`, and let managed mode restyle

  Four additions for hosts building formatting UI around drawings (web only):

  - **`onSelectionChange`** fires with the selected `Drawing` and its `rect` — the
    bounds in CSS px relative to the chart container — and again whenever that rect
    moves. The rect is recomputed from live core state once per painted frame, so a
    floating toolbar anchored to it tracks the drawing through pans, zooms, resizes
    and drags rather than snapping into place when the gesture ends. Mid-drag,
    `rect` is live while `selection.drawing` still holds the last committed
    geometry, which reaches you through `onDrawingChange` on release.
  - **`fill`** on `BoxDrawing` paints the rectangle's interior beneath the stroke.
    Note that vroom reads 8-digit hex as `#aarrggbb`, not CSS's `#rrggbbaa` — a
    green at 33% is `'#5400ce2c'`. Boxes without a `fill` are unchanged.
  - **`locked`** on any drawing keeps it selectable — so a toolbar can offer to
    unlock it — while blocking drags, reshapes and delete. Its grab handles are no
    longer drawn, since nothing about it can be grabbed.
  - **`restyle(id, patch)`** on the object published through `historyRef`, for
    changing a drawing's appearance in managed mode (where the chart owns the
    drawings array). It records one undo step and persists like any other edit.
    That type is now named `DrawingControls`; `UndoRedoControls` remains as a
    deprecated alias.

  Drawings created by a gesture now carry their stroke `color` and `width`
  explicitly instead of leaving them unset, so a host color swatch shows what is
  actually on screen. Rendering is unchanged.

  `react-native-vroom-chart` bundles the C++ core, so it takes a patch for the
  rebuilt binary; drawings remain web-only there.

## 0.11.0

### Minor Changes

- 7ccad07: **Price labels now follow the asset's own scale instead of the tick interval.**
  Deriving decimals from the interval got both ends of the range wrong: a
  five-figure asset printed `84000` where it should read `84,000.00`, and a
  sub-cent one printed `0.000060` where the digits that distinguish it start at
  the seventh decimal.

  Precision is now a fixed five significant digits with a two-decimal floor, keyed
  to the latest close, and prices carry thousands separators:

  | Price       | Renders as    |
  | ----------- | ------------- |
  | 0.000044094 | `0.000044094` |
  | 0.023397    | `0.023397`    |
  | 2513.92     | `2,513.92`    |
  | 80285.20    | `80,285.20`   |

  Because it keys to the asset rather than the visible range, the precision holds
  steady while you pan and zoom rather than shifting under your finger. The y-axis
  strip measures itself against the resulting labels, so it widens to fit nine
  decimals and stays narrow for two. The tick interval keeps one job: a zoom deep
  enough to make adjacent labels collide still adds decimals.

  This applies everywhere a price is drawn — axis labels, the current-price
  indicator, the crosshair badge, and price-line labels.

### Patch Changes

- 7ccad07: **The line chart's tip dot no longer pins itself to the right edge when you pan
  into history.** It anchored to the newest _visible_ close, so scrolling back
  dragged the marker along with the viewport instead of leaving it on the candle
  it belongs to. It now anchors to the newest close in the series and simply
  scrolls off the pane with it, matching the current-price indicator.
- 7ccad07: **The podspec no longer bakes an absolute path into your Podfile.lock.** It
  resolved the Skia source directory and wrote it straight into
  `HEADER_SEARCH_PATHS`, so the spec CocoaPods serializes to `Pods/Local
Podspecs/` — and the `SPEC CHECKSUMS` entry hashed from it — carried the
  checkout root of whichever machine last ran `pod install`. CI checks out
  somewhere else, computes a different checksum, and `pod install --deployment`
  refuses to continue:

  ```
  [!] There were changes to the lockfile in deployment mode:
  SPEC CHECKSUMS:
    react-native-vroom-chart:
      New Lockfile: d1dab5b91f8d3fc6baf8d06533c5e13a7587dce9
      Old Lockfile: 5666f29f67dbbba991df5fe7fa107c07b39aae5b
  ```

  No amount of relocking fixed it, because the committed checksum could only ever
  match one machine.

  The search paths are now emitted relative to `$(PODS_TARGET_SRCROOT)`, computed
  when the spec is evaluated rather than hardcoded, so they stay correct under
  hoisted and non-hoisted `node_modules` layouts alike. If you are carrying a
  local `yarn patch` for this, you can drop it.

## 0.10.0

### Minor Changes

- 35be804: Require `@shopify/react-native-skia` ≥ 2.11 and `react-native-worklets`
  ≥ 0.7. Construct the system font manager through public Skia APIs instead
  of RN-Skia's private HostObject context, and wrap pictures with the
  NativeState helpers 2.11 introduced. Android now links `libskia.a` from
  `react-native-skia-android`.

## 0.9.1

### Patch Changes

- ce6b4a9: **Short series no longer stretch candles to fill the plot.** `defaultCandleWidth`
  is a target body width, not a starting hint: if there aren't enough bars to fill
  the window, empty slots sit to the left of the first candle and each bar stays
  that many pixels wide. Asset switches onto a 9-candle daily series no longer
  draw ~120px-wide bars. Panning that view is a no-op (candles stay right-aligned)
  instead of snapping the bars to the left edge and locking.

  Y-axis labels also pick their decimal places from the tick interval, so a
  0.00–0.04 range no longer prints duplicate `0.01`s, and sub-cent prices
  (including ~1e-7) stay distinguishable.

## 0.9.0

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

## 0.8.0

### Minor Changes

- 456ad01: **Interval switches now animate on React Native**, matching web. Swapping the
  `candles` prop for a different interval of the same asset reshapes each candle
  into its counterpart in the new data instead of jumping, and the axis labels
  cross-fade with it.

  The animation itself has been in the C++ core since 0.7.0, but the JSI handle
  never exposed the methods that drive it, so mobile always snapped. The handle
  gains the seven that were web-only: `getVisibleRange`, `resetView`,
  `resetPriceScale`, `getVisiblePriceEnvelope`, `preservePriceEnvelope`,
  `beginIntervalMorph` and `setIntervalMorph`.

  With those in place the chart now classifies a `candles` change the way the web
  component does, rather than pushing every array straight through:

  - a **streaming** update (appended bar, updated last bar, rolling buffer) leaves
    the viewport exactly where it is
  - a **timeframe switch** re-anchors the time window so each candle keeps its
    pixel width, scale-locks a manually zoomed price axis so the candle envelope
    keeps its pixel height, and morphs into the new shape
  - a **different asset** resets to the default view

  `transitionMs` and `transitionEasing` drive the morph, so `transitionMs={0}`
  snaps. An OS reduced-motion preference now also snaps — previously it was
  honored on web only, and it now applies to the candle↔line switch and the
  volume-bar collapse on mobile too.

  `seriesKey` is the explicit escape hatch for an ambiguous switch (two assets
  trading at similar prices), same as on web. `classifyTransition`, `inferStepMs`
  and `timeframeWindow` are exported for consumers driving their own data flow.

  Apps that worked around the snap by remounting the chart on interval change
  (a `key` prop) should drop the remount — a fresh handle can't morph.

## 0.7.0

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
