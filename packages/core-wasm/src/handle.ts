// The low-level contract for one chart instance on the web.
//
// This mirrors the C facade (packages/core/include/vroom/vroom_chart.h) and the
// native JSI handle (packages/react-native/src/jsi.d.ts) — but WITHOUT the
// SkPicture return values. On web the core owns the <canvas> and its rendering
// surface, so mutations are `void` and `present()` paints. Backed by the
// Skia-WASM module (./wasm).

/** Theme color slots — mirrors the `VroomColorKey` enum in the C facade. */
export enum ColorKey {
  Background = 0,
  Bull = 1,
  Bear = 2,
  Wick = 3,
  Grid = 4,
  AxisText = 5,
  Crosshair = 6,
  TooltipBg = 7,
  TooltipText = 8,
  CrosshairTarget = 9,
  BorderBull = 10,
  BorderBear = 11,
  WickBull = 12,
  WickBear = 13,
  AccentBull = 14,
  AccentBear = 15,
}

/** OHLCV readout for the candle under the crosshair. */
export type CrosshairCandle = {
  timeMs: number;
  open: number;
  high: number;
  low: number;
  close: number;
  volume: number;
};

/**
 * What the crosshair currently snaps to. `timeMs` is the snapped slot's
 * period-start time — set even in the empty space ahead of the most recent
 * candle. `candle` holds the OHLCV when a real candle sits at that slot, and is
 * null when the crosshair is parked on a future candle-aligned slot.
 */
export type CrosshairInfo = {
  timeMs: number;
  candle: CrosshairCandle | null;
};

/** Axis region sizes in CSS px, for hit-testing gestures on the JS side. */
export type AxisMetrics = {
  yAxisWidth: number;
  xAxisHeight: number;
  /** Below-chart indicator pane height; 0 when no pane is shown. */
  indicatorHeight: number;
};

/** A moving-average overlay line, in the core's numeric encoding. */
export type OverlaySpec = {
  /** 0 = SMA, 1 = EMA. */
  kind: number;
  period: number;
  /** 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4 */
  source: number;
  /** Packed 0xAARRGGBB. */
  color: number;
  /** Stroke width in px. */
  width: number;
};

/**
 * A single chart instance bound to a canvas. Method semantics match the native
 * handle one-to-one (see jsi.d.ts) so the two platforms behave identically.
 */
export interface VroomChartHandle {
  /** Replace the full candle series (packed buffer; see packCandles). */
  setCandles(packed: ArrayBuffer): void;
  /** Set the drawing size in CSS px plus the device pixel ratio. */
  setSize(width: number, height: number, dpr: number): void;
  /** Override one theme color. `argb` is packed 0xAARRGGBB. */
  setColor(key: ColorKey | number, argb: number): void;
  /** Pass 0, 0 to show all candles. */
  setVisibleRange(startMs: number, endMs: number): void;

  /** Shift the visible range by dx/dy CSS px. */
  pan(dx: number, dy: number): void;
  /** Shift the time window and price bounds without rescaling. */
  translate(dx: number, dy: number): void;
  /** Directional zoom by per-axis factors around focus (fx, fy) in px. */
  zoom(scaleX: number, scaleY: number, fx: number, fy: number): void;
  /** Drag-on-y-axis price scaling; dy>0 widens the price range. */
  scalePriceAxis(dy: number): void;
  /** Drag-on-x-axis time scaling; dx>0 widens the time window. */
  scaleTimeAxis(dx: number): void;
  /**
   * Drag the separator between the price pane and the below-chart indicator
   * band. dy>0 (drag down) grows the price pane and shrinks the indicator band,
   * preserving candle pixel scale (the price range adjusts instead).
   */
  resizeIndicatorPane(dy: number): void;
  /**
   * Scale the y-axis of the below-chart indicator pane containing `y` (px).
   * dy>0 (drag down) zooms out (widens the value range); dy<0 zooms in. RSI
   * scales about 50, MACD about its zero line. No-op when `y` is not over a pane.
   */
  scaleIndicatorAxis(y: number, dy: number): void;

  /** Current axis dimensions for hit-testing in JS gestures. */
  getAxisMetrics(): AxisMetrics;

  /** Show the crosshair at (x, y) CSS px (y already lifted above the pointer). */
  setCrosshair(x: number, y: number): void;
  /** Hide the crosshair. */
  clearCrosshair(): void;
  /** OHLCV of the candle the crosshair snaps to, or null when inactive. */
  getCrosshairCandle(): CrosshairCandle | null;
  /**
   * The slot the crosshair snaps to (real candle or future candle-aligned
   * slot), or null when inactive. Unlike getCrosshairCandle this reports a
   * timeMs even in the empty space ahead of the most recent candle.
   */
  getCrosshairInfo(): CrosshairInfo | null;

  setRSI(
    enabled: boolean,
    period: number,
    upperBand: number,
    lowerBand: number,
    maEnabled: boolean,
    maPeriod: number,
  ): void;
  setMACD(enabled: boolean, fast: number, slow: number, signal: number): void;
  setOverlays(overlays: OverlaySpec[]): void;
  setVWAP(
    enabled: boolean,
    resetOffsetMin: number,
    color: number,
    width: number,
  ): void;

  /** True while any axis-label fade is in progress (drives the rAF loop). */
  isAnimating(): boolean;
  /** Paint the current state to the canvas. */
  present(): void;
  /** Release resources; the handle is unusable afterward. */
  destroy(): void;
}

/** Factory for chart instances. The loaded core module implements this. */
export interface VroomModule {
  /** Create a chart bound to `canvas`, attaching its rendering surface. */
  create(canvas: HTMLCanvasElement): VroomChartHandle;
}
