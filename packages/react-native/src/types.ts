import type { StyleProp, ViewStyle } from 'react-native';

export type Candle = {
  timeMs: number;
  open: number;
  high: number;
  low: number;
  close: number;
  volume: number;
};

export type CrosshairEvent = {
  /** True while the crosshair is showing; false when it's dismissed. */
  active: boolean;
  /** OHLCV of the candle under the crosshair, or null when inactive. */
  candle: Candle | null;
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

// Colors are hex strings ('#0d1117', or 8-digit '#aarrggbb') or a packed ARGB
// number. Every field is optional — omitted colors keep the library default.
export type VroomColor = string | number;

export type VroomTheme = {
  /** Chart + axis-strip background. */
  background?: VroomColor;
  /** Up candles (also bull wicks, bull volume bars, rising price indicator). */
  bull?: VroomColor;
  /** Down candles (also bear wicks, bear volume bars, falling price indicator). */
  bear?: VroomColor;
  /** Gridlines. */
  grid?: VroomColor;
  /** Axis label text (price + time). */
  axisText?: VroomColor;
  /** Crosshair dashed lines. */
  crosshair?: VroomColor;
  /** Crosshair target — the hollow ring/dot at the intersection. */
  crosshairTarget?: VroomColor;
};

export type VisibleRange = {
  startMs: number;
  endMs: number;
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

export type VroomChartProps = {
  candles: Candle[];
  /**
   * Explicit size overrides in logical px. When omitted, the chart fills its
   * parent (measured via onLayout). Use `style` (flex / aspectRatio / absolute
   * fill) for layout-driven sizing instead of hard-coding these.
   */
  width?: number;
  height?: number;
  /** Style for the chart's root view. Defaults to filling the parent. */
  style?: StyleProp<ViewStyle>;
  /** Time window to render. Omit (or both 0) to show every candle. */
  visibleRange?: VisibleRange;
  theme?: VroomTheme;
  /** RSI indicator (pane below the candles). Omit/disable to hide it. */
  rsi?: RSIConfig;
  /** MACD indicator (its own pane below the candles). Omit/disable to hide it. */
  macd?: MACDConfig;
  /** Moving-average overlay lines (SMA/EMA) drawn on the price pane. */
  movingAverages?: MovingAverageOverlay[];
  /**
   * Pixels the crosshair dot / horizontal line sit *above* the touch point so
   * they aren't hidden under the thumb. The vertical line stays centered on the
   * touch x. Default 40.
   */
  crosshairOffset?: number;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};
