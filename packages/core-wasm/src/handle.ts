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
  Line = 16,
}

/** Theme float slots — mirrors the `VroomFloatKey` enum in the C facade. */
export enum FloatKey {
  CandleWidthRatio = 0,
  WickWidth = 1,
  RightPadding = 2,
  AxisFontSize = 3,
  YAxisWidthRatio = 4,
  XAxisHeight = 5,
  VolumeOpacity = 6,
  IndicatorHeightFrac = 7,
  CandleRadius = 8,
  WickRoundCap = 9,
  VolumeRadius = 10,
  LineWidth = 11,
  LineGradientOpacity = 12,
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
  /** Free price at the crosshair's horizontal line (the price-badge value). */
  price: number;
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

/** The Bollinger Bands overlay, in the core's numeric encoding. */
export type BollingerSpec = {
  enabled: boolean;
  period: number;
  /** Standard-deviation multiplier. */
  mult: number;
  /** 0=close,1=open,2=high,3=low,4=hl2,5=hlc3,6=ohlc4 */
  source: number;
  /** Basis MA type: 0 = SMA, 1 = EMA. */
  basisKind: number;
  /** Packed 0xAARRGGBB. */
  upperColor: number;
  /** Stroke width in px. */
  upperWidth: number;
  middleColor: number;
  middleWidth: number;
  lowerColor: number;
  lowerWidth: number;
  /** Translucent fill between the bands. */
  fillEnabled: boolean;
  /** 0..1, multiplied into upperColor's alpha. */
  fillOpacity: number;
};

/**
 * A committed line drawing in the core's numeric encoding. Endpoints are in data
 * space (epoch ms + price) so the line tracks the candles on pan/zoom.
 */
export type DrawingSpec = {
  aTime: number;
  aPrice: number;
  bTime: number;
  bPrice: number;
  /** Packed 0xAARRGGBB. */
  color: number;
  /** Stroke width in px. */
  width: number;
  /**
   * Geometry: 0 = line (a→b), 1 = box (a and b are opposite corners),
   * 2 = pencil (a freehand path — see `points`).
   */
  kind: number;
  /**
   * The full path for a pencil stroke (kind 2), in draw order. Ignored for line
   * and box, which are fully described by the a/b anchors.
   */
  points?: { timeMs: number; price: number }[];
};

/** A single resting-liquidity band in the core's numeric encoding. */
export type BandSpec = {
  minPrice: number;
  maxPrice: number;
  /** 0 = buy, 1 = sell. */
  side: number;
  volume: number;
};

/** Liquidity overlay bands + shared style, in the core's numeric encoding. */
export type LiquiditySpec = {
  bands: BandSpec[];
  /** Packed 0xAARRGGBB. */
  buyColor: number;
  /** Packed 0xAARRGGBB. */
  sellColor: number;
  /** Volume mapped to peak opacity; 0 = auto-scale to the largest band. */
  maxVolume: number;
  minOpacity: number;
  maxOpacity: number;
  /** Leftward reach from the axis in px. */
  widthPx: number;
  /** Leftward reach as a fraction of pane width. */
  widthFrac: number;
};

/** Bit flags for `PriceLineSpec.flags`. */
export const PRICE_LINE_DRAGGABLE = 1 << 0;
export const PRICE_LINE_CLOSABLE = 1 << 1;
export const PRICE_LINE_AXIS_LABEL = 1 << 2;
export const PRICE_LINE_EXTEND_LEFT = 1 << 3;

/** A single price status line in the core's numeric encoding. */
export type PriceLineSpec = {
  price: number;
  /** Packed 0xAARRGGBB. */
  color: number;
  /** Stroke width in px. */
  width: number;
  /** 0 = solid, 1 = dotted, 2 = dashed. */
  lineStyle: number;
  /** Body label. Empty hides the body pill. */
  text: string;
  /** Trailing solid-fill segment. Empty hides it. */
  quantity: string;
  /** Bitwise-or of the PRICE_LINE_* flags. */
  flags: number;
};

/** Price status lines + shared style, in the core's numeric encoding. */
export type PriceLinesSpec = {
  lines: PriceLineSpec[];
  /** Translucent body/close-button pill fill, packed 0xAARRGGBB. */
  bodyBg: number;
  /** 0 = inherit the axis font size. */
  fontSizePx: number;
  /** 0..1 of pane width: the label group's right-edge inset from the axis. */
  lineLengthFrac: number;
  /** 0 = left, 1 = center, 2 = right. */
  align: number;
  /** Brightness multiplier for the hovered segment; 1 = no highlight. */
  hoverBoost: number;
};

/** A continuous data coordinate at a pixel position (no candle snapping). */
export type Coord = {
  timeMs: number;
  price: number;
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
  /** Override one theme float (e.g. wick width). */
  setFloat(key: FloatKey | number, value: number): void;
  /** Pass 0, 0 to show all candles. */
  setVisibleRange(startMs: number, endMs: number): void;
  /**
   * Target candle body width in px for the initial/reset framing. Larger = more
   * zoomed in; smaller = more zoomed out. 0 restores the default (~80 candles).
   * Affects initial framing only; an explicit setVisibleRange takes precedence.
   */
  setDefaultCandleWidth(px: number): void;
  /** Render mode: 0 = candlesticks (default), 1 = line chart (close polyline). */
  setChartType(mode: number): void;
  /**
   * Candle↔line morph blend for animated transitions. `collapse` folds candles
   * toward their close; `fade` crossfades candles→line. Both 0 = candles,
   * 1 = line. Driven per-frame by the host; setChartType snaps both.
   */
  setMorph(collapse: number, fade: number): void;
  // TODO(react-native): mirror getVisibleRange/resetView/resetPriceScale/
  // getVisiblePriceEnvelope/preservePriceEnvelope/beginIntervalMorph/
  // setIntervalMorph on the JSI handle — currently web-only.
  /** The current visible time window. {startMs: 0, endMs: 0} = uninitialized. */
  getVisibleRange(): { startMs: number; endMs: number };
  /**
   * Reset to the fresh-mount view: frame the most recent ~80 candles and
   * re-enable continuous y auto-fit (the price range follows the visible
   * candles until the next manual y gesture). Use when the data series is
   * wholesale replaced — e.g. switching assets.
   */
  resetView(): void;
  /**
   * Re-enable continuous y auto-fit only; the time window is untouched. Use
   * after repositioning the window for a same-asset data swap (e.g. a
   * timeframe switch) so the price scale re-fits the newly visible candles.
   */
  resetPriceScale(): void;
  /**
   * The visible price *envelope* — the min low / max high across the currently
   * visible candles, i.e. the extent the candles occupy rather than the (wider)
   * axis range. Null when no candles are visible.
   */
  getVisiblePriceEnvelope(): { low: number; high: number } | null;
  /**
   * Scale lock for a same-asset data swap that re-buckets the same price action
   * into a different high-low span (a timeframe switch). Rescales a *manual*
   * price range so the visible envelope keeps the exact pixel height and
   * position the given pre-swap envelope had — so candles don't suddenly shrink
   * or grow when the interval changes.
   *
   * Call after setCandles + setVisibleRange, passing the envelope read by
   * getVisiblePriceEnvelope before the swap. A no-op in auto-y mode (auto-fit is
   * already span-invariant); falls back to resetPriceScale when either envelope
   * is degenerate.
   */
  preservePriceEnvelope(prevLow: number, prevHigh: number): void;
  /**
   * Capture the visible candle geometry so the next data swap can animate as a
   * reshape rather than a jump: each candle's wick and body slide and stretch
   * into the shape of its counterpart in the new data.
   *
   * Candles are paired by *slot* — position counting back from the right edge of
   * the visible window, which a timeframe switch preserves. Call before
   * setCandles, then drive setIntervalMorph from 0 to 1.
   */
  beginIntervalMorph(): void;
  /**
   * Advance the interval morph started by beginIntervalMorph. `t` (clamped to
   * 0..1) is the eased progress: 0 renders the captured geometry pixel-
   * identically to the pre-swap frame, 1 renders the new candles and releases
   * the capture. Driven per-frame by the host animation loop.
   */
  setIntervalMorph(t: number): void;

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
  /**
   * Show the crosshair at a data-space position (epoch ms + price) instead of
   * pixels — used to mirror one chart's crosshair onto another (cross-chart
   * sync). Converts to pixels via this chart's own window/price scale; the
   * vertical line snaps to the candle for `timeMs`, the horizontal line sits at
   * `price`. Does not emit any callback.
   */
  setCrosshairData(timeMs: number, price: number): void;
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
  setBollinger(spec: BollingerSpec): void;

  /** Replace the full set of committed line drawings. Pass [] to clear. */
  setDrawings(drawings: DrawingSpec[]): void;
  /**
   * Replace the resting-liquidity overlay (bands + shared style). Pass a spec
   * with an empty `bands` array to clear the overlay.
   */
  setLiquidity(liquidity: LiquiditySpec): void;
  /**
   * Replace the full set of price status lines (plus their shared style). Pass a
   * spec with an empty `lines` array to clear them.
   */
  setPriceLines(priceLines: PriceLinesSpec): void;
  /**
   * Hit-test pixel (x, y) against the price lines. `part` is 0 for the line or
   * its label body (the drag target) and 1 for the close button; close buttons
   * win over lines and the nearest in y wins among several candidates. Only
   * draggable lines report part 0 and only closable lines report part 1. Null on
   * a miss.
   */
  hitTestPriceLine(x: number, y: number): { index: number; part: number } | null;
  /**
   * Mark a price line's segment as hovered so it renders highlighted. Pass -1 to
   * clear. `part` matches `hitTestPriceLine`.
   */
  setPriceLineHover(index: number, part: number): void;
  /**
   * Drive the live drag preview: the line, its label and its badge render at
   * `price` with a ghost at the committed one. Pass -1 to end the preview. The
   * committed price is untouched — restate `setPriceLines` to apply the move.
   */
  setPriceLineDrag(index: number, price: number): void;
  /**
   * Set the transient in-progress draft shown while placing a drawing. Node A is
   * always shown; node B is shown when `hasB`. `guide` also draws the live
   * preview (a guideline for a line, a preview rectangle for a box); when false
   * only the node dots show (the committed shape renders via setDrawings).
   * `kind` (0 = line, 1 = box) selects the preview geometry. `color`/`width`
   * style the preview.
   */
  setDraft(
    aTime: number,
    aPrice: number,
    hasB: boolean,
    bTime: number,
    bPrice: number,
    guide: boolean,
    color: number,
    width: number,
    kind: number,
  ): void;
  /**
   * Begin a freehand (pencil) draft stroke, clearing any previous draft. Follow
   * with `appendDraftPoint` per captured sample; the growing path renders live.
   */
  startDraftStroke(color: number, width: number): void;
  /**
   * Append one point to the in-progress freehand draft. Cheap enough to call on
   * every pointer move — the whole path is never restated.
   */
  appendDraftPoint(timeMs: number, price: number): void;
  /** Clear the draft (hide in-progress node dots / guideline / stroke). */
  clearDraft(): void;
  /**
   * Shift a whole committed drawing by a *relative* data-space delta — both
   * anchors and, for a pencil, every path point. Used for live body dragging.
   */
  translateDrawing(index: number, dTimeMs: number, dPrice: number): void;
  /**
   * Select a committed drawing (renders its endpoint handles). `index` -1
   * clears the selection; `grabbedEndpoint` 0/1 renders that handle 50% larger
   * while dragging (-1 for none).
   */
  setSelectedDrawing(index: number, grabbedEndpoint: number): void;
  /** Move one endpoint (0=A, 1=B) of a committed drawing to a new data anchor. */
  moveDrawingEndpoint(index: number, endpoint: number, timeMs: number, price: number): void;
  /**
   * Hit-test pixel (x, y) against the committed drawings. Returns the drawing
   * `index`, `part`, and `t`, or null on a miss. For a line, `part` is 0
   * (endpoint A), 1 (endpoint B), or 2 (body) and `t` is the 0..1 grab position
   * along the segment. For a box, `part` is 0..3 (the four corners) or 4 (body)
   * and `t` is 0. Corner/endpoint hits are only reported for the selected
   * drawing.
   */
  hitTestDrawing(x: number, y: number): { index: number; part: number; t: number } | null;
  /**
   * The continuous data coordinate at pixel (x, y) — not snapped to a candle
   * slot. Null when there are no candles or the viewport is degenerate.
   */
  coordAt(x: number, y: number): Coord | null;
  /**
   * Pixel position of a data coordinate — the inverse of coordAt. Null when
   * there are no candles or the viewport is degenerate.
   */
  project(timeMs: number, price: number): { x: number; y: number } | null;

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
