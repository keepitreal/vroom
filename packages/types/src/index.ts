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
  /** Corner radius (px) of the *top* of volume bars. Defaults to 0 (square). */
  volumeRadius?: number;
  /** Gridlines. */
  grid?: VroomColor;
  /** Axis label text (price + time). */
  axisText?: VroomColor;
  /** Crosshair dashed lines. */
  crosshair?: VroomColor;
  /** Crosshair target — the hollow ring/dot at the intersection. */
  crosshairTarget?: VroomColor;
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

/** Active drawing tool while in `draw` mode. `null` draws nothing. */
export type DrawTool = null | 'line';

/** A drawing anchor in data space, so it stays glued to the candles on pan/zoom. */
export type DrawPoint = {
  /** Anchor time as Unix epoch milliseconds (not snapped to a candle slot). */
  timeMs: number;
  /** Anchor price. */
  price: number;
};

/**
 * A committed drawing. Pass an array of these via the `drawings` prop to render
 * persisted annotations; the chart appends a new one (via `onDrawingComplete`)
 * each time the user finishes drawing. For now only the `'line'` (two-point
 * trendline) type exists.
 */
export type Drawing = {
  /** Stable unique id (the chart generates one for drawings it creates). */
  id: string;
  type: 'line';
  /** The two endpoints, in data space. */
  points: [DrawPoint, DrawPoint];
  /** Line color (hex string or packed ARGB number). Default solid blue. */
  color?: VroomColor;
  /** Stroke width in px. Default 2. */
  width?: number;
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
  theme?: VroomTheme;
  /** RSI indicator (pane below the candles). Omit/disable to hide it. */
  rsi?: RSIConfig;
  /** MACD indicator (its own pane below the candles). Omit/disable to hide it. */
  macd?: MACDConfig;
  /** Moving-average overlay lines (SMA/EMA) drawn on the price pane. */
  movingAverages?: MovingAverageOverlay[];
  /** VWAP overlay (session anchor, configurable reset). */
  vwap?: VWAPConfig;
  /** Resting-order / order-book liquidity bands drawn behind the candles. */
  liquidity?: LiquidityConfig;
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
   */
  drawings?: Drawing[];
  /** Fired with the finished drawing when the user completes one. */
  onDrawingComplete?: (drawing: Drawing) => void;
  /**
   * Fired when the chart wants the mode changed — e.g. it requests `'pan'` after
   * the user clicks away from a just-drawn line. Since `mode` is controlled, the
   * host should apply the requested mode.
   */
  onModeChange?: (mode: ChartMode) => void;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};
