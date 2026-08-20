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
  /**
   * Overrides a single theme float. `key` is a VroomFloatKey index (see
   * FLOAT_KEYS in theme.ts); e.g. wick width.
   */
  setFloat(key: number, value: number): void;
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
  /** Candle↔line morph blend: collapse folds candles to close, fade crossfades. */
  setMorph(collapse: number, fade: number): void;
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
   * Configures the RSI pane. maKind mirrors setOverlays' kind encoding; colors
   * are packed 0xAARRGGBB where 0 means inherit, and a non-positive width
   * inherits the default stroke.
   */
  setRSI(spec: {
    enabled: boolean;
    period: number;
    upperBand: number;
    lowerBand: number;
    maPeriod: number;
    maKind: number;
    maVisible: boolean;
    lineColor: number;
    lineWidth: number;
    lineVisible: boolean;
    maColor: number;
    maWidth: number;
    bandColor: number;
    bandsVisible: boolean;
  }): void;
  /**
   * Configures the MACD pane. source/maKind mirror setOverlays' encodings;
   * colors are packed 0xAARRGGBB where 0 means inherit, and a non-positive
   * width inherits the default stroke.
   */
  setMACD(spec: {
    enabled: boolean;
    fast: number;
    slow: number;
    signal: number;
    source: number;
    maKind: number;
    signalMaKind: number;
    lineColor: number;
    lineWidth: number;
    lineVisible: boolean;
    signalColor: number;
    signalWidth: number;
    signalVisible: boolean;
    histVisible: boolean;
    histUpColor: number;
    histUpFadingColor: number;
    histDownColor: number;
    histDownFadingColor: number;
    zeroColor: number;
    zeroVisible: boolean;
  }): void;
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
  setVWAP(spec: {
    enabled: boolean;
    resetOffsetMin: number;
    color: number;
    width: number;
  }): void;
  /**
   * Configures the Bollinger Bands overlay (three price-pane lines + optional
   * fill between the bands). source/basisKind mirror setOverlays' encodings;
   * colors are packed 0xAARRGGBB; fillOpacity is 0..1.
   */
  setBollinger(spec: {
    enabled: boolean;
    period: number;
    mult: number;
    source: number;
    basisKind: number;
    upperColor: number;
    upperWidth: number;
    middleColor: number;
    middleWidth: number;
    lowerColor: number;
    lowerWidth: number;
    fillEnabled: boolean;
    fillOpacity: number;
  }): void;
  /**
   * Configures the volume bars under the candles. `heightFrac` is the tallest
   * bar as a fraction of the price pane. The style fields carry an inherit
   * sentinel: a negative number or a transparent color falls back to the
   * matching theme key.
   */
  setVolume(spec: {
    enabled: boolean;
    heightFrac: number;
    opacity: number;
    radiusPx: number;
    upColor: number;
    downColor: number;
  }): void;
  /**
   * Staggered volume-bar collapse: 0 = full height, 1 = all bars flat. Bars fall
   * tallest-first and land together; drive 1 → 0 to reveal them, which plays the
   * cascade in reverse (shortest bar home first).
   *
   * Unlike setMorph, `t` must be **linear** progress — the core eases each bar
   * over its own slice of the timeline, so the curve is applied there. `easing`
   * indexes `linear | ease-in | ease-out | ease-in-out`. setVolume snaps this to
   * match its `enabled`, so it's only needed while animating.
   */
  setVolumeCollapse(t: number, easing: number): void;
  /**
   * The continuous data coordinate at pixel (x, y) — not snapped to a candle
   * slot. Null when there are no candles or the viewport is degenerate. Cheap to
   * call at gesture rate (no rendering).
   */
  coordAt(x: number, y: number): { timeMs: number; price: number } | null;
  /**
   * Replaces the full set of price status lines (plus their shared style).
   * lineStyle: 0=solid,1=dotted,2=dashed; color/bodyBg are packed 0xAARRGGBB;
   * flags is a bitmask (1=draggable, 2=closable, 4=axis label, 8=extend left);
   * align: 0=left,1=center,2=right. Pass an empty `lines` array to clear.
   */
  setPriceLines(spec: {
    lines: {
      price: number;
      color: number;
      width: number;
      lineStyle: number;
      text: string;
      quantity: string;
      flags: number;
    }[];
    bodyBg: number;
    fontSizePx: number;
    lineLengthFrac: number;
    align: number;
    hoverBoost: number;
  }): void;
  /**
   * Hit-tests pixel (x, y) against the price lines. `part` is 0 for the line or
   * its label body (the drag target) and 1 for the close button; close buttons
   * win over lines and the nearest in y wins among several candidates. Only
   * draggable lines report part 0 and only closable lines report part 1. Cheap
   * to call at gesture rate (no rendering).
   */
  hitTestPriceLine(x: number, y: number): { index: number; part: number } | null;
  /**
   * Marks a price line's segment as hovered so it renders highlighted; -1 clears.
   * `part` matches hitTestPriceLine. Touch has no hover state, so this is here
   * for pointer devices and web parity.
   */
  setPriceLineHover(index: number, part: number): void;
  /**
   * Drives the live drag preview: the line, its label and its badge render at
   * `price` with a ghost at the committed one. Pass -1 to end the preview. The
   * committed price is untouched — restate setPriceLines to apply the move.
   */
  setPriceLineDrag(index: number, price: number): void;
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
