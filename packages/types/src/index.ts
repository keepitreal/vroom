// Shared, platform-agnostic types for the vroom chart public API.
//
// Both `react-native-vroom-chart` (native) and `@vroomchart/react` (web) re-export
// these and add their own platform-specific `style` typing on top, so the two
// components present one identical API surface.

/** A single OHLCV bar. `candles` is an array of these. */
export type Candle = {
  /** Bar open time as Unix epoch milliseconds. */
  timeMs: number;
  /** Opening price. */
  open: number;
  /** Highest price during the bar. */
  high: number;
  /** Lowest price during the bar. */
  low: number;
  /** Closing price. */
  close: number;
  /** Traded volume during the bar. */
  volume: number;
};

/** Payload passed to `onCrosshair` as the crosshair shows, moves, or hides. */
export type CrosshairEvent = {
  /** True while the crosshair is showing; false when it's dismissed. */
  active: boolean;
  /**
   * OHLCV of the candle under the crosshair, or null when inactive. Also null
   * when the crosshair is parked on a *future* candle-aligned slot in the empty
   * space ahead of the most recent candle (no candle exists there yet) — use
   * `timeMs` to read the slot's time in that case.
   */
  candle: Candle | null;
  /**
   * Bar-open time (Unix epoch ms) of the slot the crosshair snaps to, including
   * future candle-aligned slots past the last candle. Null when inactive.
   */
  timeMs: number | null;
  /**
   * Free price at the crosshair's horizontal line (what the price badge shows),
   * in data space. Null when inactive. Pair with `timeMs` to mirror this
   * crosshair onto another chart via its `crosshairOverride` prop.
   */
  price: number | null;
  /**
   * Why this event fired — lets the host react differently (e.g. haptics):
   *   'show' — long-press activated the crosshair
   *   'move' — the crosshair moved: a different candle (time) or a vertical move
   *            within the same candle (price). Fires on any positional change so
   *            `price` stays current for cross-chart sync; for a per-candle
   *            signal (e.g. haptics), dedupe on `timeMs` yourself.
   *   'hide' — the crosshair was dismissed
   * The library never plays haptics itself; the host decides.
   */
  reason: 'show' | 'move' | 'hide';
};

/**
 * A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
 * packed ARGB number. In `VroomTheme` every field is optional — omitted colors
 * keep the library default.
 */
export type VroomColor = string | number;

/** Color overrides for the chart, passed via the `theme` prop. */
export type VroomTheme = {
  /** Chart + axis-strip background. */
  background?: VroomColor;
  /** Up candle body fill. Wick and border default to this unless overridden. */
  bull?: VroomColor;
  /** Down candle body fill. Wick and border default to this unless overridden. */
  bear?: VroomColor;
  /** Generic up color for the price indicator, volume bars, and MACD histogram. Defaults to teal-green; independent of `bull`. */
  accentBull?: VroomColor;
  /** Generic down color for the price indicator, volume bars, and MACD histogram. Defaults to red; independent of `bear`. */
  accentBear?: VroomColor;
  /** Up candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bull fill color to hide it. */
  borderBull?: VroomColor;
  /** Down candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bear fill color to hide it. */
  borderBear?: VroomColor;
  /** Up candle wick color. Defaults to the bull fill color. */
  wickBull?: VroomColor;
  /** Down candle wick color. Defaults to the bear fill color. */
  wickBear?: VroomColor;
  /** Wick stroke width in px (applies to both up and down wicks). Defaults to 1. */
  wickWidth?: number;
  /** Corner radius (px) of candle bodies. Defaults to 0 (square). */
  candleRadius?: number;
  /** Round the wick end caps. Defaults to false. */
  wickRoundCap?: boolean;
  /**
   * Corner radius (px) of the *top* of volume bars. Defaults to 0 (square).
   *
   * @deprecated Use `volume.radius`, which sits with the rest of the volume
   * styling. This still applies when `volume.radius` is omitted.
   */
  volumeRadius?: number;
  /** Gridlines. */
  grid?: VroomColor;
  /** Axis label text (price + time). */
  axisText?: VroomColor;
  /** Crosshair dashed lines. */
  crosshair?: VroomColor;
  /** Crosshair target — the hollow ring/dot at the intersection. */
  crosshairTarget?: VroomColor;
  /** Line-chart-mode close polyline color. Defaults to violet, matching the RSI line. */
  lineColor?: VroomColor;
  /** Line-chart-mode polyline stroke width in px. Defaults to 1.5. */
  lineWidth?: number;
  /**
   * Opacity of the gradient filled beneath the line-chart polyline, at its
   * strongest point. The fill uses `lineColor` and ramps to fully transparent at
   * the bottom of the price pane. Defaults to 0.28; set to 0 to disable the fill.
   */
  lineGradientOpacity?: number;
};

/** A time window over the candle data, as Unix epoch milliseconds. */
export type VisibleRange = {
  /** Window start (inclusive), Unix epoch milliseconds. */
  startMs: number;
  /** Window end (inclusive), Unix epoch milliseconds. */
  endMs: number;
};

/**
 * Chart interaction mode.
 *   'pan'  — default: drag to pan, pinch/wheel to zoom, hover/long-press crosshair.
 *   'draw' — left-clicks place drawing points; panning/zooming are suppressed.
 */
export type ChartMode = 'pan' | 'draw';

/**
 * How the price series is drawn.
 *   'candles' — default: candlestick bodies + wicks.
 *   'line'    — a single polyline through each candle's close. Volume, indicators,
 *               overlays, crosshair, and drawings still render.
 */
export type ChartType = 'candles' | 'line';

/**
 * Easing curve for animated transitions (candle↔line and interval switches).
 * Defaults to `'ease-in-out'`.
 */
export type TransitionEasing = 'linear' | 'ease-in' | 'ease-out' | 'ease-in-out';

/** Active drawing tool while in `draw` mode. `null` draws nothing. */
export type DrawTool = null | 'line' | 'box' | 'pencil';

/** A drawing anchor in data space, so it stays glued to the candles on pan/zoom. */
export type DrawPoint = {
  /** Anchor time as Unix epoch milliseconds (not snapped to a candle slot). */
  timeMs: number;
  /** Anchor price. */
  price: number;
};

/** Fields shared by every drawing type. */
type DrawingBase = {
  /** Stable unique id (the chart generates one for drawings it creates). */
  id: string;
  /** Stroke color (hex string or packed ARGB number). Default solid blue. */
  color?: VroomColor;
  /** Stroke width in px. Default 2. */
  width?: number;
};

/** A two-point trendline from `points[0]` to `points[1]`. */
export type LineDrawing = DrawingBase & {
  type: 'line';
  /** The two endpoints, in data space. */
  points: [DrawPoint, DrawPoint];
};

/**
 * An axis-aligned rectangle whose two opposite corners are `points[0]` and
 * `points[1]` (the other two corners are derived).
 */
export type BoxDrawing = DrawingBase & {
  type: 'box';
  /** Two opposite corners, in data space. */
  points: [DrawPoint, DrawPoint];
};

/**
 * A freehand pencil stroke: an open path through `points`, in order. Unlike the
 * other tools a stroke has a variable number of points, and once committed it
 * can only be translated — never reshaped.
 */
export type PencilDrawing = DrawingBase & {
  type: 'pencil';
  /** The path's points in draw order (at least 2), in data space. */
  points: DrawPoint[];
};

/**
 * A committed drawing. Pass an array of these via the `drawings` prop to render
 * persisted annotations; the chart appends a new one (via `onDrawingComplete`)
 * each time the user finishes drawing.
 *
 * This is a discriminated union on `type` — narrow on it before reading
 * `points[1]`, since a `'pencil'` stroke has a variable-length array while
 * `'line'` and `'box'` are always exactly two points.
 */
export type Drawing = LineDrawing | BoxDrawing | PencilDrawing;

/**
 * Storage adapter for **managed** drawing persistence. Provide it via the
 * `drawingStore` prop and the chart owns the drawings array itself — loading and
 * saving through this adapter instead of you wiring the controlled `drawings`
 * prop + `onDrawing*` callbacks.
 *
 * The adapter is an **opaque string key-value store** — the chart serializes
 * drawings into a **versioned envelope** (`{ v, drawings }`) and hands you the
 * string; you just persist bytes. Because the library owns the schema and
 * migrates old payloads on load, adding drawing tools or persisted fields later
 * never changes this interface — your adapter is written once.
 *
 * `marketId` is the chart's `seriesKey`, so drawings are bucketed per market:
 * they persist across timeframe changes (same key) but not across markets. Both
 * methods may be async (localStorage is sync; AsyncStorage / MMKV / a REST
 * backend are async). The chart debounces `save`. Consumers that only handle a
 * single market can ignore `marketId`.
 */
export type DrawingStore = {
  /**
   * Return the raw string previously handed to `save` for `marketId`, or
   * `null`/`undefined`/`''` if nothing is stored. Sync or async.
   */
  load: (marketId: string) => string | null | undefined | Promise<string | null | undefined>;
  /**
   * Persist the opaque `data` string for `marketId`. Sync or async; the chart
   * debounces calls. The string is a versioned envelope owned by the library —
   * store it verbatim, don't parse or reshape it.
   */
  save: (marketId: string, data: string) => void | Promise<void>;
};

/**
 * Whether undo/redo are currently available for the chart's drawings — the
 * payload of `onHistoryChange`, for binding toolbar button enabled-states.
 */
export type UndoRedoState = {
  canUndo: boolean;
  canRedo: boolean;
};

/**
 * Programmatic undo/redo controls, published through the `historyRef` prop in
 * managed mode — for toolbar buttons and other UI outside the chart. The
 * keyboard shortcuts (⌘Z / ⇧⌘Z / Ctrl+Y) work without this.
 */
export type UndoRedoControls = {
  /** Roll back the most recent committed drawing action. No-op when empty. */
  undo: () => void;
  /** Re-apply the most recently undone action. No-op when empty. */
  redo: () => void;
  /**
   * Drop the undo/redo stacks without touching the drawings. Rarely needed —
   * users find a cleared history surprising — but useful when reusing a mounted
   * chart for what the user perceives as a brand-new context.
   */
  clearHistory: () => void;
};

/** RSI indicator config. Rendered in a pane below the candles when enabled. */
export type RSIConfig = {
  enabled?: boolean;
  /** Lookback period in candle counts. Default 14, clamped to >= 2. */
  period?: number;
  /** Overbought band level (0..100). Default 70. */
  upperBand?: number;
  /** Oversold band level (0..100). Default 30. */
  lowerBand?: number;
  /** Show the RSI-based moving-average trendline. Default true. */
  maEnabled?: boolean;
  /** Trendline (MA of RSI) length. Default 14, clamped to >= 1. */
  maPeriod?: number;
};

/** Price source for a moving average. */
export type MASource =
  | 'close'
  | 'open'
  | 'high'
  | 'low'
  | 'hl2'
  | 'hlc3'
  | 'ohlc4';

/**
 * A moving-average overlay line drawn on the price pane. Provide an array of
 * these via `movingAverages` to render a ribbon of SMA/EMA lines.
 */
export type MovingAverageOverlay = {
  /** 'sma' (simple) or 'ema' (exponential). */
  kind: 'sma' | 'ema';
  /** Lookback in candles. */
  length: number;
  /** Price source. Default 'close'. */
  source?: MASource;
  /** Line color (hex string or packed ARGB number). */
  color?: string | number;
  /** Stroke width in px. Default 1.5. */
  width?: number;
};

/**
 * VWAP overlay config (session anchor). Drawn as a single line on the price
 * pane, resetting each session.
 */
export type VWAPConfig = {
  enabled?: boolean;
  /** Session reset offset from UTC midnight, in minutes (default 0). */
  resetMinutes?: number;
  /** Line color (hex string or packed ARGB number). */
  color?: string | number;
  /** Stroke width in px. Default 1.5. */
  width?: number;
};

/**
 * Bollinger Bands overlay config. A basis moving average of `source` over
 * `period`, banded at ± `stdDev` × population standard deviation of the same
 * window, drawn as three lines on the price pane with an optional translucent
 * fill between the bands. No pane is reserved.
 */
export type BollingerBandsConfig = {
  enabled?: boolean;
  /** Lookback in candles. Default 20, clamped to >= 1. */
  period?: number;
  /** Standard-deviation multiplier. Default 2. */
  stdDev?: number;
  /** Price source. Default 'close'. */
  source?: MASource;
  /**
   * Basis (middle) line type. Default 'sma'. The stdev always uses the
   * window's arithmetic mean, even with an EMA basis (the standard semantics).
   */
  basis?: 'sma' | 'ema';
  /** Upper band color (hex string or packed ARGB number). Default blue. */
  upperColor?: string | number;
  /** Upper band stroke width in px. Default 1. */
  upperWidth?: number;
  /** Basis (middle) line color. Default orange. */
  middleColor?: string | number;
  /** Basis line stroke width in px. Default 1. */
  middleWidth?: number;
  /** Lower band color. Default blue. */
  lowerColor?: string | number;
  /** Lower band stroke width in px. Default 1. */
  lowerWidth?: number;
  /** Translucent fill between the bands. Default true. */
  fill?: boolean;
  /** Fill opacity 0..1, applied to the upper band color. Default 0.1. */
  fillOpacity?: number;
};

/**
 * Volume bar config. One bottom-anchored bar per candle on the price pane,
 * drawn under the candles and sharing their x position and body width.
 *
 * Unlike the other indicator configs the bars are on by default, so omitting
 * this prop leaves the chart looking as it always has.
 */
export type VolumeConfig = {
  /** Draw the bars. Default true. */
  enabled?: boolean;
  /** Bar opacity 0..1 (1 = opaque). Default 0.5, so bars read quieter than the candles. */
  opacity?: number;
  /**
   * Height of the tallest bar as a fraction of the price pane, 0..1.
   * Default 0.2.
   *
   * This is a ceiling rather than a reserved strip: raising it lets the bars
   * reach further up over the candles rather than compressing them, matching
   * the conventional volume overlay. Heights always auto-fit the loudest volume
   * in view, so the tallest bar sits exactly at the ceiling.
   */
  height?: number;
  /** Corner radius (px) of the *top* of each bar. Defaults to `theme.volumeRadius`, else 0 (square). */
  radius?: number;
  /** Up-bar color (hex string or packed ARGB number). Defaults to `theme.accentBull`. */
  upColor?: string | number;
  /** Down-bar color (hex string or packed ARGB number). Defaults to `theme.accentBear`. */
  downColor?: string | number;
};

/**
 * A single resting-liquidity band: a price interval carrying a total order size
 * on one side of the book. Consolidate raw L2 levels into these buckets before
 * passing them. The vertical extent is defined in price space, so bands scale
 * with the y-axis on zoom.
 */
export type LiquidityBand = {
  /** Bottom of the price interval. */
  minPrice: number;
  /** Top of the price interval. A per-level order uses a thin interval. */
  maxPrice: number;
  /** Which side of the book — selects the buy vs sell color. */
  side: 'buy' | 'sell';
  /** Total resting size in this band; drives the band's opacity. */
  volume: number;
};

/**
 * Resting-order / order-book liquidity overlay. Each band is drawn as a
 * horizontal rectangle anchored at the inner edge of the price axis, stretching
 * left (~`widthPx`/`widthFrac` of the pane) and fading out with a gradient,
 * colored by side and made more opaque with volume. Rendered behind the candles.
 * Omit the prop (or pass no bands) to hide the overlay.
 */
export type LiquidityConfig = {
  /** The bands to render. */
  bands: LiquidityBand[];
  /** Buy-side color (hex string or packed ARGB number). Default teal-green. */
  buyColor?: VroomColor;
  /** Sell-side color (hex string or packed ARGB number). Default red. */
  sellColor?: VroomColor;
  /**
   * Volume mapped to the peak (`maxOpacity`) band. Omit to auto-scale to the
   * largest band's volume each render.
   */
  maxVolume?: number;
  /** Opacity floor for the smallest band. Default 0.05. */
  minOpacity?: number;
  /** Opacity at `maxVolume`. Default 0.8. */
  maxOpacity?: number;
  /** Leftward reach from the axis in px. Default 300. */
  widthPx?: number;
  /** Leftward reach as a fraction of pane width. Default 0.25. The smaller of
   * `widthPx` and `widthFrac * paneWidth` is used. */
  widthFrac?: number;
};

/**
 * A consumer-supplied horizontal status line at a fixed price — the primitive
 * behind resting limit orders, take-profits, stop-losses and liquidation levels.
 *
 * Renders as a line across the price pane ending in a label group (a `text` pill
 * plus an optional solid-filled `quantity` pill and an optional close button),
 * with a price badge in the y-axis strip.
 *
 * Interaction is opt-in and callback-gated: the line is only draggable when
 * `draggable` is set, and the close button only renders when you pass
 * `onPriceLineClose`. Dragging is a *preview* — the chart never mutates
 * the price you gave it, so a move your backend rejects reverts on its own
 * simply by leaving your `priceLines` state unchanged.
 */
export type PriceLine = {
  /** Stable unique id, echoed back by every callback. */
  id: string;
  /** Where the line sits on the price scale. */
  price: number;
  /** Body label — render whatever you like (e.g. `'Limit Buy @ 13.79'`). */
  text?: string;
  /**
   * Trailing segment, drawn as a solid-filled pill with white text so size reads
   * at a glance (e.g. `'x 5.206'`). Omit to hide it.
   */
  quantity?: string;
  /** Line, border, body text and close-icon color. Defaults to a soft red. */
  color?: VroomColor;
  /** Stroke width in px. Default 1. */
  width?: number;
  /** Line style. Default `'dotted'`, matching the current-price indicator. */
  lineStyle?: 'solid' | 'dotted' | 'dashed';
  /**
   * Let the user drag this line vertically to a new price. Default false.
   * Pair with `onPriceLineDragEnd` to commit the move.
   */
  draggable?: boolean;
  /**
   * Show the close button on this line. Defaults to true, but the button only
   * ever renders if you also pass `onPriceLineClose` — set this to false to opt a
   * single line out (e.g. a liquidation level the user can't dismiss).
   */
  closable?: boolean;
  /** Extend the line to the pane's left edge. Default true. */
  extendLeft?: boolean;
  /** Show the price badge in the y-axis strip. Default true. */
  axisLabel?: boolean;
};

/** Shared layout/style for every price line, passed via `priceLinesStyle`. */
export type PriceLinesStyle = {
  /**
   * Translucent fill behind the body and close-button pills, so the label reads
   * over candles without hiding them. Defaults to a dark translucent grey.
   */
  bodyBackground?: VroomColor;
  /** Label font size in px. Defaults to the axis font size. */
  fontSize?: number;
  /**
   * How far in from the price axis the label group sits, as a fraction of pane
   * width (0 = flush against the axis, 0.5 = at the pane's midpoint). Only
   * applies when `align` is `'right'`. Default 0.
   */
  inset?: number;
  /** Where the label group sits horizontally. Default `'right'`. */
  align?: 'left' | 'center' | 'right';
  /**
   * How much the hovered line or close button brightens, as a channel
   * multiplier. 1 disables the highlight. Default 1.25. Web only — touch
   * platforms have no hover state.
   */
  hoverBoost?: number;
};

/** MACD indicator config. Rendered in its own pane below the candles. */
export type MACDConfig = {
  enabled?: boolean;
  /** Fast EMA length. Default 12. */
  fast?: number;
  /** Slow EMA length (forced > fast). Default 26. */
  slow?: number;
  /** Signal-line EMA length. Default 9. */
  signal?: number;
};

/**
 * Platform-agnostic props shared by every vroom chart component. Each platform
 * extends this with its own `style` typing (and any platform-only props) to
 * form its public `VroomChartProps`.
 */
export type VroomChartCoreProps = {
  /** OHLCV bars to render. The only required prop. */
  candles: Candle[];
  /**
   * Identity of the data series (e.g. "BTC-USD"). When it changes between
   * renders the chart resets to the default view (most recent candles + price
   * auto-fit), regardless of what the data heuristics conclude — the escape
   * hatch for ambiguous switches like two assets trading at similar prices.
   * Omit to rely on automatic detection from the candle data alone.
   */
  seriesKey?: string;
  /**
   * Explicit size overrides in logical px. When omitted, the chart fills its
   * parent (measured at runtime). Prefer layout-driven sizing via the
   * platform's `style` (flex / aspect-ratio / absolute fill) over hard-coding.
   */
  width?: number;
  height?: number;
  /** Time window to render. Omit (or both 0) to show every candle. */
  visibleRange?: VisibleRange;
  /**
   * Target candle *body* width in logical px for the INITIAL framing. Larger →
   * more zoomed in (fewer candles); smaller → more zoomed out. Only affects
   * first load / reset; an explicit `visibleRange` takes precedence. Omit for
   * the default (~80 candles). Useful to keep candles a consistent size across
   * devices of different widths.
   */
  defaultCandleWidth?: number;
  /**
   * Price-series render style. `'candles'` (default) draws candlesticks;
   * `'line'` draws a polyline through each candle's close (style it with
   * `theme.lineColor` / `theme.lineWidth`). All other layers are unaffected.
   */
  chartType?: ChartType;
  /**
   * Duration (ms) of the animated transitions: the candle↔line switch when
   * `chartType` changes, and the candle reshape when the `candles` array is
   * swapped for a different interval of the same asset. Default ~300. `0` snaps
   * instantly. Ignored (snaps) when the OS requests reduced motion, which
   * instead uses a plain cross-fade.
   */
  transitionMs?: number;
  /**
   * Easing curve applied to those transitions. Default `'ease-in-out'`.
   */
  transitionEasing?: TransitionEasing;
  theme?: VroomTheme;
  /** RSI indicator (pane below the candles). Omit/disable to hide it. */
  rsi?: RSIConfig;
  /** MACD indicator (its own pane below the candles). Omit/disable to hide it. */
  macd?: MACDConfig;
  /** Moving-average overlay lines (SMA/EMA) drawn on the price pane. */
  movingAverages?: MovingAverageOverlay[];
  /** VWAP overlay (session anchor, configurable reset). */
  vwap?: VWAPConfig;
  /** Bollinger Bands overlay (three lines + fill on the price pane). */
  bollingerBands?: BollingerBandsConfig;
  /** Volume bars under the candles. On by default; disable or restyle them here. */
  volume?: VolumeConfig;
  /** Resting-order / order-book liquidity bands drawn behind the candles. */
  liquidity?: LiquidityConfig;
  /**
   * Horizontal status lines at consumer-supplied prices (resting orders, TP/SL,
   * liquidation levels), each with a text label and an optional close button.
   *
   * This is a controlled prop: the chart renders exactly what you pass and never
   * mutates it. A drag paints a live preview and reports the new price through
   * `onPriceLineDragEnd`; the line only actually moves once you update this
   * array, so a rejected move needs no explicit rollback.
   */
  priceLines?: PriceLine[];
  /** Shared layout/style for every entry in `priceLines`. */
  priceLinesStyle?: PriceLinesStyle;
  /**
   * Fired continuously while a draggable price line is being dragged, with the
   * price under the pointer. Use it for a live readout (e.g. an order ticket);
   * wait for `onPriceLineDragEnd` to commit.
   */
  onPriceLineDrag?: (id: string, price: number) => void;
  /**
   * Fired once when the user drops a dragged price line. Submit the new price to
   * your backend here, then update `priceLines` on success — the line snaps back
   * on its own if you don't. Dragging is cancelled (and this never fires) if the
   * user presses Escape.
   */
  onPriceLineDragEnd?: (id: string, price: number) => void;
  /**
   * Fired when the user activates a price line's close button. Passing this
   * handler is what makes the button render at all; individual lines opt out with
   * `closable: false`.
   */
  onPriceLineClose?: (id: string) => void;
  /**
   * Pixels the crosshair dot / horizontal line sit *above* the touch point so
   * they aren't hidden under the thumb. The vertical line stays centered on the
   * touch x. Default 40.
   */
  crosshairOffset?: number;
  /**
   * Controlled external crosshair, in data space (epoch ms + price). When
   * non-null the chart shows a crosshair there — typically synced from another
   * chart's `onCrosshair` — *without* emitting its own `onCrosshair`, so two
   * charts can drive each other without a feedback loop. A local hover/press on
   * this chart takes precedence; on release the override is restored. Set null
   * to clear. Best when the synced charts share a comparable price scale (the
   * horizontal line clamps to the pane edge when the price is off-scale).
   */
  crosshairOverride?: { timeMs: number; price: number } | null;
  /**
   * Interaction mode. Default `'pan'`. Set to `'draw'` (with a `tool`) to let
   * the user place drawing points; panning/zooming are suppressed while drawing.
   */
  mode?: ChartMode;
  /** Active drawing tool while in `draw` mode. Default `null` (draws nothing). */
  tool?: DrawTool;
  /**
   * Committed drawings to render, anchored to data so they track the candles on
   * pan/zoom. This is a controlled prop: append the value the chart hands you in
   * `onDrawingComplete` to persist it.
   *
   * Ignored when `drawingStore` is set (the chart then owns the array itself).
   */
  drawings?: Drawing[];
  /**
   * Opt into **managed** drawing persistence: the chart owns the drawings array
   * internally and loads/saves it through this adapter, keyed by `seriesKey`.
   * When set, `drawings` and the `onDrawing*` callbacks are ignored. Web only.
   * Requires `seriesKey` — without one, drawings work in-session but aren't saved.
   */
  drawingStore?: DrawingStore;
  /** Fired with the finished drawing when the user completes one. */
  onDrawingComplete?: (drawing: Drawing) => void;
  /**
   * Fired after the user drags a selected line's endpoint handle. The payload is
   * the same drawing (same `id`) with updated `points`; apply it to your
   * controlled `drawings` state (replace by id). Web only.
   */
  onDrawingChange?: (drawing: Drawing) => void;
  /**
   * Fired when the user deletes the selected line (Backspace/Delete). Remove the
   * drawing with this `id` from your controlled `drawings` state. Web only.
   */
  onDrawingDelete?: (id: string) => void;
  /**
   * Fired when the chart wants the mode changed — e.g. it requests `'pan'` after
   * the user clicks away from a just-drawn line. Since `mode` is controlled, the
   * host should apply the requested mode.
   */
  onModeChange?: (mode: ChartMode) => void;
  /**
   * Max drawing undo depth in managed mode (one step = one committed drawing
   * action). Oldest steps are evicted beyond this. Default 100. History is
   * in-memory and per-`seriesKey`: it resets on market switch and is never
   * persisted — only the drawings themselves are saved. Web only.
   */
  historyLimit?: number;
  /**
   * Fired when drawing undo/redo availability changes in managed mode — bind
   * toolbar undo/redo buttons' enabled-state to it. (In controlled mode you own
   * the history, so track availability yourself.) Web only.
   */
  onHistoryChange?: (state: UndoRedoState) => void;
  /**
   * Receives programmatic `undo`/`redo`/`clearHistory` controls in managed mode
   * (e.g. `useRef<UndoRedoControls | null>(null)` passed here, then
   * `historyRef.current?.undo()` from a toolbar button). Set to `null` while
   * unmounted or when no `drawingStore` is present. Web only.
   */
  historyRef?: { current: UndoRedoControls | null };
  /**
   * Fired when the user presses the undo shortcut (⌘Z / Ctrl+Z) in controlled
   * mode — apply the undo to your own drawings state. Ignored when
   * `drawingStore` is set (managed mode undoes internally). Web only.
   */
  onUndo?: () => void;
  /**
   * Fired when the user presses the redo shortcut (⇧⌘Z / Ctrl+Shift+Z /
   * Ctrl+Y) in controlled mode — apply the redo to your own drawings state.
   * Ignored when `drawingStore` is set. Web only.
   */
  onRedo?: () => void;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};
