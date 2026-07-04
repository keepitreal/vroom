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
   * Why this event fired — lets the host react differently (e.g. haptics):
   *   'show' — long-press activated the crosshair
   *   'move' — the crosshair snapped to a *different* candle (one per candle
   *            crossed; not per drag frame)
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
  /** Up candle body 1px border. Defaults to the bull fill color. */
  borderBull?: VroomColor;
  /** Down candle body 1px border. Defaults to the bear fill color. */
  borderBear?: VroomColor;
  /** Up candle wick color. Defaults to the bull fill color. */
  wickBull?: VroomColor;
  /** Down candle wick color. Defaults to the bear fill color. */
  wickBear?: VroomColor;
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
  theme?: VroomTheme;
  /** RSI indicator (pane below the candles). Omit/disable to hide it. */
  rsi?: RSIConfig;
  /** MACD indicator (its own pane below the candles). Omit/disable to hide it. */
  macd?: MACDConfig;
  /** Moving-average overlay lines (SMA/EMA) drawn on the price pane. */
  movingAverages?: MovingAverageOverlay[];
  /** VWAP overlay (session anchor, configurable reset). */
  vwap?: VWAPConfig;
  /**
   * Pixels the crosshair dot / horizontal line sit *above* the touch point so
   * they aren't hidden under the thumb. The vertical line stays centered on the
   * touch x. Default 40.
   */
  crosshairOffset?: number;
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
