// Ambient declaration for the JSI HostObject installed by VroomChartModule.

import type { SkPicture } from '@shopify/react-native-skia';

export interface ChartHandle {
  setCandles(buffer: ArrayBuffer): void;
  setSize(width: number, height: number, pxRatio: number): void;
  /**
   * Overrides a single theme color. `key` is a VroomColorKey index (see
   * COLOR_KEYS in theme.ts); `argb` is a packed 0xAARRGGBB integer.
   */
  setColor(key: number, argb: number): void;
  /** Pass 0, 0 to show all candles. */
  setVisibleRange(startMs: number, endMs: number): void;
  /** Shifts the visible range by `dx`/`dy` pixels and returns a fresh picture. */
  pan(dx: number, dy: number): SkPicture | null;
  /**
   * Two-finger translation: shifts the time window AND the price bounds
   * without rescaling. dy > 0 (drag down) moves content down.
   */
  translate(dx: number, dy: number): SkPicture | null;
  /**
   * Directional zoom by per-axis multiplicative factors around focus point
   * (`fx`, `fy`) in pixels. `scaleX` resizes the time window (>1 = wider
   * candles); `scaleY` resizes the price range (>1 = taller candles). Pass 1
   * for an axis to leave it untouched.
   */
  zoom(scaleX: number, scaleY: number, fx: number, fy: number): SkPicture | null;
  /**
   * Drag-on-y-axis price scaling. `dy > 0` widens the price range
   * (candles shrink). Pivots around the price-range center.
   */
  scalePriceAxis(dy: number): SkPicture | null;
  /**
   * Drag-on-x-axis time scaling. `dx > 0` widens the time window
   * (candles thin). Pivots around the right edge so the most recent
   * visible candle stays in place.
   */
  scaleTimeAxis(dx: number): SkPicture | null;
  /**
   * Current axis dimensions in pixels for hit testing in JS gestures.
   * `indicatorHeight` is the below-chart indicator pane height (0 when none).
   */
  getAxisMetrics(): {
    yAxisWidth: number;
    xAxisHeight: number;
    indicatorHeight: number;
  };
  /**
   * Shows the crosshair at (`x`, `y`) in pixels and returns a fresh picture.
   * `y` should already be lifted above the touch point so the dot/horizontal
   * line aren't hidden under the thumb.
   */
  setCrosshair(x: number, y: number): SkPicture | null;
  /** Hides the crosshair and returns a fresh picture. */
  clearCrosshair(): SkPicture | null;
  /**
   * OHLCV of the candle the crosshair currently snaps to, or null when the
   * crosshair is inactive / there are no visible candles. Cheap to poll at
   * gesture rate (no rendering). Call after setCrosshair to read the candle
   * under the new position.
   */
  getCrosshairCandle(): {
    timeMs: number;
    open: number;
    high: number;
    low: number;
    close: number;
    volume: number;
  } | null;
  /**
   * The slot the crosshair currently snaps to, or null when the crosshair is
   * inactive / there are no visible candles. Unlike getCrosshairCandle, this
   * reports a `timeMs` even in the empty space ahead of the most recent candle;
   * `candle` is null when the crosshair is parked on such a future slot. Cheap
   * to poll at gesture rate (no rendering).
   */
  getCrosshairInfo(): {
    timeMs: number;
    candle: {
      timeMs: number;
      open: number;
      high: number;
      low: number;
      close: number;
      volume: number;
    } | null;
  } | null;
  /**
   * Configures the RSI pane: enable, period (>=2), overbought/oversold band
   * levels (0..100), and the RSI-based MA trendline (toggle + length >=1).
   */
  setRSI(
    enabled: boolean,
    period: number,
    upperBand: number,
    lowerBand: number,
    maEnabled: boolean,
    maPeriod: number,
  ): void;
  /**
   * Configures the MACD pane: enable, fast/slow EMA lengths (slow forced > fast)
   * and the signal-line length. Defaults 12/26/9.
   */
  setMACD(enabled: boolean, fast: number, slow: number, signal: number): void;
  /**
   * Replaces the full set of MA/EMA overlay lines drawn on the price pane.
   * kind: 0=SMA, 1=EMA; source: 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4;
   * color: packed 0xAARRGGBB; width: stroke px.
   */
  setOverlays(
    overlays: {
      kind: number;
      period: number;
      source: number;
      color: number;
      width: number;
    }[],
  ): void;
  /**
   * Configures the session VWAP overlay. `resetOffsetMin` shifts the session
   * boundary from UTC midnight (minutes); `color` is packed 0xAARRGGBB.
   */
  setVWAP(
    enabled: boolean,
    resetOffsetMin: number,
    color: number,
    width: number,
  ): void;
  /** True while any axis-label fade is still in progress. Drives a RAF loop. */
  isAnimating(): boolean;
  render(): SkPicture | null;
}

export interface VroomChartJSI {
  /** Creates a fresh chart instance. Destroyed when the JS reference is GC'd. */
  create(): ChartHandle;
}

declare global {
  // eslint-disable-next-line no-var
  var VroomChartJSI: VroomChartJSI | undefined;
}

// eslint-disable-next-line @typescript-eslint/no-empty-interface
export {};
