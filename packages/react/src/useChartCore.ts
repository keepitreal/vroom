// Owns one VroomChartHandle bound to a <canvas>: loads the core, measures the
// container (ResizeObserver), pushes data/size/theme/indicators on change, and
// drives a rAF-batched present() loop. The web analogue of
// packages/react-native/src/useChartCore.ts — but it paints the canvas via
// handle.present() instead of producing an SkPicture.

import { useCallback, useEffect, useRef, useState } from 'react';
import {
  loadVroom,
  packCandles,
  applyTheme,
  parseColor,
  FloatKey,
  type LoadVroomOptions,
  type OverlaySpec,
  type BollingerSpec,
  type MACDSpec,
  type RSISpec,
  type VWAPSpec,
  type VolumeSpec,
  type DrawingSpec,
  type LiquiditySpec,
  type PriceLinesSpec,
  type VroomChartHandle,
  PRICE_LINE_AXIS_LABEL,
  PRICE_LINE_CLOSABLE,
  PRICE_LINE_DRAGGABLE,
  PRICE_LINE_EXTEND_LEFT,
} from '@vroomchart/core-wasm';
import type { TransitionEasing, VroomChartCoreProps } from '@vroomchart/types';
import {
  classifyTransition,
  inferStepMs,
  timeframeWindow,
  type DataTransition,
} from './dataTransitions';
import { ease, easingIndex } from './easing';
import { isValidDrawing } from './drawingStorage';

function prefersReducedMotion(): boolean {
  return !!(
    typeof window !== 'undefined' &&
    window.matchMedia &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches
  );
}

// Mirrors vroom::ma::Source order (packages/core/src/ma.h).
const MA_SOURCES = ['close', 'open', 'high', 'low', 'hl2', 'hlc3', 'ohlc4'] as const;

// An unset style color marshals as the core's transparent inherit sentinel.
const inheritColor = (v: string | number | undefined): number =>
  (v != null ? parseColor(v) : null) ?? 0;

function overlayToNumeric(
  o: NonNullable<VroomChartCoreProps['movingAverages']>[number],
): OverlaySpec {
  const srcIdx = o.source ? MA_SOURCES.indexOf(o.source) : 0;
  return {
    kind: o.maType === 'ema' ? 1 : 0,
    period: o.period,
    source: srcIdx < 0 ? 0 : srcIdx,
    color: (o.color != null ? parseColor(o.color) : null) ?? 0xff2962ff,
    width: o.width ?? 1.5,
  };
}

function rsiToSpec(cfg: VroomChartCoreProps['rsi']): RSISpec {
  return {
    enabled: cfg?.enabled ?? false,
    period: cfg?.period ?? 14,
    upperBand: cfg?.upperBand ?? 70,
    lowerBand: cfg?.lowerBand ?? 30,
    maPeriod: cfg?.maPeriod ?? 14,
    maKind: cfg?.maType === 'ema' ? 1 : 0,
    maVisible: cfg?.maVisible ?? true,
    lineColor: inheritColor(cfg?.lineColor),
    lineWidth: cfg?.lineWidth ?? -1,
    lineVisible: cfg?.lineVisible ?? true,
    maColor: inheritColor(cfg?.maColor),
    maWidth: cfg?.maWidth ?? -1,
    bandColor: inheritColor(cfg?.bandColor),
    bandsVisible: cfg?.bandsVisible ?? true,
  };
}

function vwapToSpec(cfg: VroomChartCoreProps['vwap']): VWAPSpec {
  return {
    enabled: cfg?.enabled ?? false,
    resetOffsetMin: cfg?.resetMinutes ?? 0,
    color: inheritColor(cfg?.color),
    width: cfg?.width ?? -1,
  };
}

// Bollinger defaults: blue bands / orange basis, matching the repo palette.
const DEFAULT_BB_BAND_COLOR = 0xff2962ff;
const DEFAULT_BB_BASIS_COLOR = 0xffff6d00;

function bollingerToSpec(
  cfg: VroomChartCoreProps['bollingerBands'],
): BollingerSpec {
  const srcIdx = cfg?.source ? MA_SOURCES.indexOf(cfg.source) : 0;
  return {
    enabled: cfg?.enabled ?? false,
    period: cfg?.period ?? 20,
    mult: cfg?.stdDev ?? 2,
    source: srcIdx < 0 ? 0 : srcIdx,
    basisKind: cfg?.maType === 'ema' ? 1 : 0,
    upperColor:
      (cfg?.upperColor != null ? parseColor(cfg.upperColor) : null) ?? DEFAULT_BB_BAND_COLOR,
    upperWidth: cfg?.upperWidth ?? 1,
    middleColor:
      (cfg?.middleColor != null ? parseColor(cfg.middleColor) : null) ?? DEFAULT_BB_BASIS_COLOR,
    middleWidth: cfg?.middleWidth ?? 1,
    lowerColor:
      (cfg?.lowerColor != null ? parseColor(cfg.lowerColor) : null) ?? DEFAULT_BB_BAND_COLOR,
    lowerWidth: cfg?.lowerWidth ?? 1,
    fillEnabled: cfg?.fillVisible ?? true,
    fillOpacity: cfg?.fillOpacity ?? 0.1,
  };
}

function macdToSpec(cfg: VroomChartCoreProps['macd']): MACDSpec {
  const srcIdx = cfg?.source ? MA_SOURCES.indexOf(cfg.source) : 0;
  return {
    enabled: cfg?.enabled ?? false,
    fast: cfg?.fast ?? 12,
    slow: cfg?.slow ?? 26,
    signal: cfg?.signal ?? 9,
    source: srcIdx < 0 ? 0 : srcIdx,
    maKind: cfg?.maType === 'sma' ? 0 : 1,
    signalMaKind: cfg?.signalMaType === 'sma' ? 0 : 1,
    lineColor: inheritColor(cfg?.lineColor),
    lineWidth: cfg?.lineWidth ?? -1,
    lineVisible: cfg?.lineVisible ?? true,
    signalColor: inheritColor(cfg?.signalColor),
    signalWidth: cfg?.signalWidth ?? -1,
    signalVisible: cfg?.signalVisible ?? true,
    histVisible: cfg?.histogramVisible ?? true,
    histUpColor: inheritColor(cfg?.histogramUpColor),
    histUpFadingColor: inheritColor(cfg?.histogramUpFadingColor),
    histDownColor: inheritColor(cfg?.histogramDownColor),
    histDownFadingColor: inheritColor(cfg?.histogramDownFadingColor),
    zeroColor: inheritColor(cfg?.zeroLineColor),
    zeroVisible: cfg?.zeroLineVisible ?? true,
  };
}

// Unset style fields go down as the core's inherit sentinels (negative float,
// transparent color) rather than as literal defaults, so the theme keys stay in
// charge of anything the consumer didn't set.
function volumeToSpec(cfg: VroomChartCoreProps['volume']): VolumeSpec {
  return {
    enabled: cfg?.enabled ?? true,
    heightFrac: cfg?.height ?? -1,
    opacity: cfg?.opacity ?? -1,
    radiusPx: cfg?.radius ?? -1,
    upColor: (cfg?.upColor != null ? parseColor(cfg.upColor) : null) ?? 0,
    downColor: (cfg?.downColor != null ? parseColor(cfg.downColor) : null) ?? 0,
  };
}

function drawingToSpec(
  d: NonNullable<VroomChartCoreProps['drawings']>[number],
): DrawingSpec {
  // a/b are the first and last anchor — the two endpoints of a line, the two
  // opposite corners of a box, or the ends of a pencil stroke or path (whose
  // full point list rides along in `points`).
  const first = d.points[0];
  const last = d.points[d.points.length - 1];
  const multiPoint = d.type === 'pencil' || d.type === 'path';
  return {
    aTime: first.timeMs,
    aPrice: first.price,
    bTime: last.timeMs,
    bPrice: last.price,
    color: (d.color != null ? parseColor(d.color) : null) ?? 0xff2962ff,
    width: d.width ?? 2,
    kind: d.type === 'box' ? 1 : d.type === 'pencil' ? 2 : d.type === 'path' ? 3 : 0,
    // 0 is the core's "unset" sentinel: a box with no fill keeps its default
    // tint rather than turning transparent.
    fill: (d.type === 'box' && d.fill != null ? parseColor(d.fill) : null) ?? 0,
    locked: d.locked ?? false,
    ...(multiPoint
      ? { points: d.points.map((p) => ({ timeMs: p.timeMs, price: p.price })) }
      : {}),
  };
}

// Default buy/sell colors (teal-green / red), matching the accent palette.
const DEFAULT_BUY_COLOR = 0xff26a69a;
const DEFAULT_SELL_COLOR = 0xffef5350;

function liquidityToSpec(
  cfg: NonNullable<VroomChartCoreProps['liquidity']>,
): LiquiditySpec {
  return {
    bands: cfg.bands.map((b) => ({
      minPrice: b.minPrice,
      maxPrice: b.maxPrice,
      side: b.side === 'sell' ? 1 : 0,
      volume: b.volume,
    })),
    buyColor: (cfg.buyColor != null ? parseColor(cfg.buyColor) : null) ?? DEFAULT_BUY_COLOR,
    sellColor:
      (cfg.sellColor != null ? parseColor(cfg.sellColor) : null) ?? DEFAULT_SELL_COLOR,
    maxVolume: cfg.maxVolume ?? 0,
    minOpacity: cfg.minOpacity ?? 0.05,
    maxOpacity: cfg.maxOpacity ?? 0.8,
    widthPx: cfg.widthPx ?? 300,
    widthFrac: cfg.widthFrac ?? 0.25,
  };
}

// Cleared overlay: an empty band set (style values are irrelevant when there are
// no bands, but the spec shape requires them).
const EMPTY_LIQUIDITY: LiquiditySpec = {
  bands: [],
  buyColor: DEFAULT_BUY_COLOR,
  sellColor: DEFAULT_SELL_COLOR,
  maxVolume: 0,
  minOpacity: 0.05,
  maxOpacity: 0.8,
  widthPx: 300,
  widthFrac: 0.25,
};

// Price-line defaults: a soft red dotted rule with a dark translucent label,
// close in weight to the current-price indicator it sits beside.
const DEFAULT_PRICE_LINE_COLOR = 0xffef5350;
const DEFAULT_PRICE_LINE_BODY_BG = 0xd91c2128;
const DEFAULT_PRICE_LINE_HOVER_BOOST = 1.25;

const LINE_STYLES = { solid: 0, dotted: 1, dashed: 2 } as const;

function priceLinesToSpec(
  lines: NonNullable<VroomChartCoreProps['priceLines']>,
  style: VroomChartCoreProps['priceLinesStyle'],
  // Whether a close handler exists. The button is callback-gated: with nothing
  // for it to do, it isn't drawn at all.
  hasCloseHandler: boolean,
): PriceLinesSpec {
  return {
    lines: lines.map((l) => ({
      price: l.price,
      color: (l.color != null ? parseColor(l.color) : null) ?? DEFAULT_PRICE_LINE_COLOR,
      width: l.width ?? 1,
      lineStyle: LINE_STYLES[l.lineStyle ?? 'dotted'],
      text: l.text ?? '',
      quantity: l.quantity ?? '',
      flags:
        (l.draggable ? PRICE_LINE_DRAGGABLE : 0) |
        (hasCloseHandler && l.closable !== false ? PRICE_LINE_CLOSABLE : 0) |
        (l.axisLabel !== false ? PRICE_LINE_AXIS_LABEL : 0) |
        (l.extendLeft !== false ? PRICE_LINE_EXTEND_LEFT : 0),
    })),
    bodyBg:
      (style?.bodyBackground != null ? parseColor(style.bodyBackground) : null) ??
      DEFAULT_PRICE_LINE_BODY_BG,
    fontSizePx: style?.fontSize ?? 0,
    lineLengthFrac: style?.inset ?? 0,
    align: style?.align === 'left' ? 0 : style?.align === 'center' ? 1 : 2,
    hoverBoost: style?.hoverBoost ?? DEFAULT_PRICE_LINE_HOVER_BOOST,
  };
}

// Cleared overlay: no lines (the style values are irrelevant, but the spec shape
// requires them).
const EMPTY_PRICE_LINES: PriceLinesSpec = {
  lines: [],
  bodyBg: DEFAULT_PRICE_LINE_BODY_BG,
  fontSizePx: 0,
  lineLengthFrac: 0,
  align: 2,
  hoverBoost: DEFAULT_PRICE_LINE_HOVER_BOOST,
};

export type UseChartCore = {
  containerRef: React.RefObject<HTMLDivElement | null>;
  canvasRef: React.RefObject<HTMLCanvasElement | null>;
  handleRef: React.RefObject<VroomChartHandle | null>;
  /** Schedules a rAF-batched repaint (and keeps ticking while animating). */
  scheduleRender: () => void;
  /**
   * Called right after each paint. Assign to observe the geometry the core just
   * drew, whatever caused it to change.
   */
  afterPresentRef: React.RefObject<(() => void) | null>;
  /** Measured CSS size of the container (0 until first layout). */
  size: { width: number; height: number };
};

export function useChartCore(
  props: VroomChartCoreProps,
  loadOpts?: LoadVroomOptions,
): UseChartCore {
  const {
    candles,
    seriesKey,
    width: widthProp,
    height: heightProp,
    visibleRange,
    defaultCandleWidth,
    chartType,
    transitionMs,
    transitionEasing,
    theme,
    rsi,
    macd,
    movingAverages,
    vwap,
    bollingerBands,
    volume,
    drawings,
    liquidity,
    priceLines,
    priceLinesStyle,
    onPriceLineClose,
  } = props;

  const containerRef = useRef<HTMLDivElement | null>(null);
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const handleRef = useRef<VroomChartHandle | null>(null);
  // What the core currently holds, for classifying the next data change.
  // Keyed by handle so a recreated core is treated as a fresh initial load.
  const prevDataRef = useRef<{
    handle: VroomChartHandle;
    candles: VroomChartCoreProps['candles'];
    seriesKey?: string;
  } | null>(null);
  const rafRef = useRef<number | null>(null);
  const intervalMorphRafRef = useRef<number | null>(null);
  // Volume-bar collapse: the last value handed to the core (null until the first
  // sync), so the data effect can restore it and a toggle mid-animation can
  // reverse from where it is. Declared up here because the data effect reads it.
  const volumeCollapseRef = useRef<number | null>(null);
  const [ready, setReady] = useState(false);
  const [measured, setMeasured] = useState({ width: 0, height: 0 });

  // Animation config in a ref, refreshed each render, so changing the duration
  // or easing doesn't re-run the data-push effect below (which would re-push
  // every candle).
  const animRef = useRef<{ ms: number; easing: TransitionEasing | undefined }>({
    ms: 300,
    easing: undefined,
  });
  animRef.current = { ms: Math.max(0, transitionMs ?? 300), easing: transitionEasing };

  // Captured at mount: which core to load (stub vs Skia-WASM). Changing it after
  // mount has no effect — the core is created once and shared process-wide.
  const loadOptsRef = useRef(loadOpts);
  loadOptsRef.current = loadOpts;

  const width = widthProp ?? measured.width;
  const height = heightProp ?? measured.height;

  // Runs right after each paint. Every visual change — pan, zoom, resize, a
  // drag in flight, a selection — reaches the screen through this one rAF, so a
  // hook here sees them all without subscribing to each individually. Used by
  // the gesture layer to report the selection's on-screen rect (which is only
  // meaningful once the frame it belongs to has been laid out).
  const afterPresentRef = useRef<(() => void) | null>(null);

  const scheduleRender = useCallback(() => {
    if (rafRef.current != null) return;
    rafRef.current = requestAnimationFrame(() => {
      rafRef.current = null;
      const h = handleRef.current;
      if (!h) return;
      h.present();
      afterPresentRef.current?.();
      if (h.isAnimating()) scheduleRender();
    });
  }, []);

  // Stop an in-flight interval morph and land the core on the new candles.
  const endIntervalMorph = useCallback(() => {
    if (intervalMorphRafRef.current != null) {
      cancelAnimationFrame(intervalMorphRafRef.current);
      intervalMorphRafRef.current = null;
    }
    handleRef.current?.setIntervalMorph(1);
  }, []);

  // Runs the interval morph clock. The core holds the pre-swap geometry (see
  // beginIntervalMorph) and reshapes each candle slot toward its new counterpart.
  const startIntervalMorph = useCallback((h: VroomChartHandle) => {
    const { ms, easing } = animRef.current;
    const start = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - start) / ms);
      h.setIntervalMorph(p < 1 ? ease(easing, p) : 1);
      h.present();
      intervalMorphRafRef.current = p < 1 ? requestAnimationFrame(step) : null;
    };
    intervalMorphRafRef.current = requestAnimationFrame(step);
  }, []);

  // Create the handle once the canvas exists; destroy on unmount.
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    let disposed = false;
    loadVroom(loadOptsRef.current).then((mod) => {
      if (disposed) return;
      handleRef.current = mod.create(canvas);
      setReady(true);
    });
    return () => {
      disposed = true;
      if (rafRef.current != null) cancelAnimationFrame(rafRef.current);
      rafRef.current = null;
      if (intervalMorphRafRef.current != null) cancelAnimationFrame(intervalMorphRafRef.current);
      intervalMorphRafRef.current = null;
      handleRef.current?.destroy();
      handleRef.current = null;
      setReady(false);
    };
  }, []);

  // Measure the container (unless both dims are pinned via props).
  useEffect(() => {
    const el = containerRef.current;
    if (!el || (widthProp != null && heightProp != null)) return;
    const ro = new ResizeObserver((entries) => {
      const r = entries[0]?.contentRect;
      if (!r) return;
      const w = Math.round(r.width);
      const h = Math.round(r.height);
      setMeasured((prev) => (prev.width === w && prev.height === h ? prev : { width: w, height: h }));
    });
    ro.observe(el);
    return () => ro.disconnect();
  }, [widthProp, heightProp]);

  // Stable deps so inline object/array literals don't re-run every render.
  const themeKey = theme ? JSON.stringify(theme) : '';
  const rsiKey = rsi ? JSON.stringify(rsi) : '';
  const macdKey = macd ? JSON.stringify(macd) : '';
  const maKey = movingAverages ? JSON.stringify(movingAverages) : '';
  const vwapKey = vwap ? JSON.stringify(vwap) : '';
  const bollingerKey = bollingerBands ? JSON.stringify(bollingerBands) : '';
  const volumeKey = volume ? JSON.stringify(volume) : '';
  const drawingsKey = drawings ? JSON.stringify(drawings) : '';
  const liquidityKey = liquidity ? JSON.stringify(liquidity) : '';
  // Whether a close handler exists is part of the rendered output (it gates the
  // button), so it belongs in the key alongside the lines themselves.
  const priceLinesKey = priceLines
    ? JSON.stringify([priceLines, priceLinesStyle ?? null, onPriceLineClose != null])
    : '';
  const explicit = visibleRange != null;
  const startMs = visibleRange?.startMs ?? 0;
  const endMs = visibleRange?.endMs ?? 0;

  // Push everything into the core whenever data/size/config changes, then paint.
  useEffect(() => {
    const h = handleRef.current;
    if (!h || width <= 0 || height <= 0) return;
    const dpr = typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1;
    h.setSize(width, height, dpr);
    if (candles.length > 0) {
      const prev = prevDataRef.current;
      const freshHandle = prev == null || prev.handle !== h;
      if (freshHandle || prev.candles !== candles || prev.seriesKey !== seriesKey) {
        // A fresh core frames itself (its window starts at 0/0); an explicit
        // visibleRange prop overrides any auto behavior, so treat the change
        // like a stream and let the range application below win.
        const transition: DataTransition = freshHandle
          ? 'initial'
          : explicit
            ? 'stream'
            : classifyTransition(prev.candles, candles, seriesKey !== prev.seriesKey);

        // Capture the outgoing view before setCandles re-infers the candle
        // period from the new data.
        let tfArgs: { oldWindow: { startMs: number; endMs: number }; oldStepMs: number; oldLastMs: number } | null =
          null;
        // The pre-swap candle envelope, used to scale-lock the y-axis below.
        let prevEnvelope: { low: number; high: number } | null = null;
        let willMorph = false;
        if (transition === 'timeframe' && prev != null) {
          const oldWindow = h.getVisibleRange();
          const oldStepMs = inferStepMs(prev.candles);
          if (oldWindow.endMs > oldWindow.startMs && oldStepMs != null) {
            tfArgs = { oldWindow, oldStepMs, oldLastMs: prev.candles[prev.candles.length - 1].timeMs };
          }
          prevEnvelope = h.getVisiblePriceEnvelope();
          // Capture the outgoing candle geometry, but only when it will actually
          // be animated so a disabled animation costs no snapshot. A switch
          // during a morph restarts from the data the core currently holds.
          willMorph = animRef.current.ms > 0 && !prefersReducedMotion();
          if (willMorph) {
            endIntervalMorph();
            h.beginIntervalMorph();
          }
        } else if (transition === 'initial' || transition === 'reset') {
          // Wholesale reframing — the slot pairing no longer holds, so land any
          // in-flight morph rather than reshaping into unrelated data.
          endIntervalMorph();
        }

        // Drive the initial zoom from a target candle width, but only on a
        // fresh handle and only when the caller isn't explicitly controlling the
        // range. Pushed before setCandles so the core's default framing (run
        // inside setCandles while the window is still 0/0) picks it up.
        if (
          freshHandle &&
          !explicit &&
          defaultCandleWidth != null &&
          defaultCandleWidth > 0
        ) {
          h.setDefaultCandleWidth(defaultCandleWidth);
        }

        h.setCandles(packCandles(candles));

        if (transition === 'timeframe') {
          const newStepMs = inferStepMs(candles);
          if (tfArgs && newStepMs != null) {
            const w = timeframeWindow(
              tfArgs.oldWindow,
              tfArgs.oldStepMs,
              tfArgs.oldLastMs,
              newStepMs,
              candles[candles.length - 1].timeMs,
            );
            h.setVisibleRange(w.startMs, w.endMs);
          }
          // Scale-lock the y-axis: the same price action re-buckets into a
          // smaller/larger high-low span, so a manual price range is rescaled to
          // keep the candle envelope at the pixel height it just had instead of
          // snapping back to auto-fit. A no-op in auto-y mode, which is already
          // span-invariant.
          if (prevEnvelope) h.preservePriceEnvelope(prevEnvelope.low, prevEnvelope.high);
          else h.resetPriceScale();
          // Started after the new bounds are in place: the snapshot is in band
          // fractions, so frame 0 still matches the pre-switch pixels exactly.
          if (willMorph) startIntervalMorph(h);
        } else if (transition === 'reset') {
          h.resetView();
        }
        prevDataRef.current = { handle: h, candles, seriesKey };
      }
    }
    if (explicit) h.setVisibleRange(startMs, endMs);
    if (theme) applyTheme(h, theme);
    // The tip dot stays, only its animation drops — the same bargain the
    // candle↔line morph strikes when it keeps the crossfade but skips the
    // collapse. Also stops the pulse from pinning a rAF loop for a user who
    // asked for less motion.
    if (prefersReducedMotion()) h.setFloat(FloatKey.LineTipPulse, 0);
    h.setRSI(rsiToSpec(rsi));
    h.setMACD(macdToSpec(macd));
    h.setOverlays((movingAverages ?? []).map(overlayToNumeric));
    h.setVWAP(vwapToSpec(vwap));
    h.setBollinger(bollingerToSpec(bollingerBands));
    h.setVolume(volumeToSpec(volume));
    // setVolume snaps the collapse scalar to its `enabled`, which would cut a
    // toggle animation short whenever this effect re-runs (a streaming candle,
    // a resize). Hand the in-flight value back. On the toggle itself this runs
    // before the driver effect below, so what lands here is the pre-toggle value
    // the animation starts from — no flash either way.
    if (volumeCollapseRef.current != null) {
      h.setVolumeCollapse(volumeCollapseRef.current, easingIndex(animRef.current.easing));
    }
    // The controlled `drawings` prop is consumer-supplied — filter malformed
    // entries (wrong arity, non-finite anchors) before they reach the WASM
    // boundary, matching the validation the managed store path already does.
    h.setDrawings((drawings ?? []).filter(isValidDrawing).map(drawingToSpec));
    h.setLiquidity(
      liquidity?.bands?.length ? liquidityToSpec(liquidity) : EMPTY_LIQUIDITY,
    );
    h.setPriceLines(
      priceLines?.length
        ? priceLinesToSpec(priceLines, priceLinesStyle, onPriceLineClose != null)
        : EMPTY_PRICE_LINES,
    );
    scheduleRender();
    // theme/rsi/macd/movingAverages/vwap/bollingerBands/volume/drawings/
    // liquidity/priceLines tracked via *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ready, width, height, candles, seriesKey, explicit, startMs, endMs, defaultCandleWidth, themeKey, rsiKey, macdKey, maKey, vwapKey, bollingerKey, volumeKey, drawingsKey, liquidityKey, priceLinesKey, scheduleRender, startIntervalMorph, endIntervalMorph]);

  // Animate the candle↔line transition when `chartType` changes. The core is
  // driven per-frame with a (collapse, fade) blend; we own the eased clock here
  // in JS (see plan) so it works identically on the web direct-draw path. A new
  // handle applies the target instantly; reduced motion / transitionMs=0 snaps
  // (reduced motion still cross-fades opacity but skips the vertical collapse).
  const morphRafRef = useRef<number | null>(null);
  const morphFadeRef = useRef<number | null>(null);
  const morphHandleRef = useRef<VroomChartHandle | null>(null);
  useEffect(() => {
    if (!ready) return;
    const h = handleRef.current;
    if (!h) return;
    const target = chartType === 'line' ? 1 : 0;

    // Fresh handle (first load or remount): snap, don't animate.
    if (morphHandleRef.current !== h || morphFadeRef.current == null) {
      morphHandleRef.current = h;
      morphFadeRef.current = target;
      h.setChartType(target);
      scheduleRender();
      return;
    }
    if (morphFadeRef.current === target) return;

    const reduce = prefersReducedMotion();
    const dur = Math.max(0, transitionMs ?? 300);

    if (morphRafRef.current != null) {
      cancelAnimationFrame(morphRafRef.current);
      morphRafRef.current = null;
    }
    if (dur === 0) {
      morphFadeRef.current = target;
      h.setChartType(target);
      scheduleRender();
      return;
    }

    const from = morphFadeRef.current;
    const start = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - start) / dur);
      const fade = from + (target - from) * ease(animRef.current.easing, p);
      morphFadeRef.current = fade;
      h.setMorph(reduce ? 0 : fade, fade);
      h.present();
      if (p < 1) {
        morphRafRef.current = requestAnimationFrame(step);
      } else {
        morphRafRef.current = null;
        morphFadeRef.current = target;
        h.setChartType(target); // lock the exact endpoint
        h.present();
      }
    };
    morphRafRef.current = requestAnimationFrame(step);

    return () => {
      if (morphRafRef.current != null) {
        cancelAnimationFrame(morphRafRef.current);
        morphRafRef.current = null;
      }
    };
  }, [ready, chartType, transitionMs, scheduleRender]);

  // Animate the volume bars in and out when `volume.enabled` flips. The core
  // staggers the bars itself — tallest falling first, all landing together — so
  // unlike the loops above this one hands it *linear* progress plus the curve;
  // pre-easing here would compound the two. Hiding drives 0→1, revealing 1→0,
  // which is the same cascade backwards.
  const volumeRafRef = useRef<number | null>(null);
  const volumeHandleRef = useRef<VroomChartHandle | null>(null);
  useEffect(() => {
    if (!ready) return;
    const h = handleRef.current;
    if (!h) return;
    const target = (volume?.enabled ?? true) ? 0 : 1;

    // Fresh handle (first load or remount): the data effect's setVolume already
    // snapped it, so a chart that mounts with bars doesn't animate them in.
    if (volumeHandleRef.current !== h || volumeCollapseRef.current == null) {
      volumeHandleRef.current = h;
      volumeCollapseRef.current = target;
      return;
    }
    if (volumeCollapseRef.current === target) return;

    if (volumeRafRef.current != null) {
      cancelAnimationFrame(volumeRafRef.current);
      volumeRafRef.current = null;
    }

    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0 || prefersReducedMotion()) {
      volumeCollapseRef.current = target;
      h.setVolumeCollapse(target, easingIndex(animRef.current.easing));
      scheduleRender();
      return;
    }

    // From wherever the last frame left off, so toggling mid-flight reverses
    // instead of jumping. A partial trip covers less ground in the same time.
    const from = volumeCollapseRef.current;
    const start = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - start) / dur);
      const t = p < 1 ? from + (target - from) * p : target;
      volumeCollapseRef.current = t;
      h.setVolumeCollapse(t, easingIndex(animRef.current.easing));
      h.present();
      volumeRafRef.current = p < 1 ? requestAnimationFrame(step) : null;
    };
    volumeRafRef.current = requestAnimationFrame(step);

    return () => {
      if (volumeRafRef.current != null) {
        cancelAnimationFrame(volumeRafRef.current);
        volumeRafRef.current = null;
      }
    };
  }, [ready, volume?.enabled, transitionMs, scheduleRender]);

  return {
    containerRef,
    canvasRef,
    handleRef,
    scheduleRender,
    afterPresentRef,
    size: { width, height },
  };
}
