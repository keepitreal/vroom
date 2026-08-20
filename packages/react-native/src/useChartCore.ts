import { useCallback, useEffect, useRef, useState } from 'react';
import type { MutableRefObject } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { DataTransition } from './dataTransitions';
import { classifyTransition, inferStepMs, timeframeWindow } from './dataTransitions';
import { ease } from './easing';
import type { ChartHandle } from './jsi.d';
import { packCandles } from './packCandles';
import { applyTheme, parseColor } from './theme';
import type {
  BollingerBandsConfig,
  Candle,
  ChartType,
  MACDConfig,
  MovingAverageOverlay,
  PriceLine,
  PriceLinesStyle,
  RSIConfig,
  TransitionEasing,
  VisibleRange,
  VolumeConfig,
  VroomTheme,
  VWAPConfig,
} from './types';

// Mirrors vroom::ma::Source order in packages/core/src/ma.h.
const MA_SOURCES = [
  'close',
  'open',
  'high',
  'low',
  'hl2',
  'hlc3',
  'ohlc4',
] as const;

// An unset style color marshals as the core's transparent inherit sentinel.
const inheritColor = (v: string | number | undefined): number =>
  (v != null ? parseColor(v) : null) ?? 0;

function overlayToNumeric(o: MovingAverageOverlay) {
  const srcIdx = o.source ? MA_SOURCES.indexOf(o.source) : 0;
  return {
    kind: o.maType === 'ema' ? 1 : 0,
    period: o.period,
    source: srcIdx < 0 ? 0 : srcIdx,
    color: (o.color != null ? parseColor(o.color) : null) ?? 0xff2962ff,
    width: o.width ?? 1.5,
  };
}

function rsiToSpec(cfg: RSIConfig | undefined) {
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

function vwapToSpec(cfg: VWAPConfig | undefined) {
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

function bollingerToSpec(cfg: BollingerBandsConfig | undefined) {
  const srcIdx = cfg?.source ? MA_SOURCES.indexOf(cfg.source) : 0;
  return {
    enabled: cfg?.enabled ?? false,
    period: cfg?.period ?? 20,
    mult: cfg?.stdDev ?? 2,
    source: srcIdx < 0 ? 0 : srcIdx,
    basisKind: cfg?.maType === 'ema' ? 1 : 0,
    upperColor:
      (cfg?.upperColor != null ? parseColor(cfg.upperColor) : null) ??
      DEFAULT_BB_BAND_COLOR,
    upperWidth: cfg?.upperWidth ?? 1,
    middleColor:
      (cfg?.middleColor != null ? parseColor(cfg.middleColor) : null) ??
      DEFAULT_BB_BASIS_COLOR,
    middleWidth: cfg?.middleWidth ?? 1,
    lowerColor:
      (cfg?.lowerColor != null ? parseColor(cfg.lowerColor) : null) ??
      DEFAULT_BB_BAND_COLOR,
    lowerWidth: cfg?.lowerWidth ?? 1,
    fillEnabled: cfg?.fillVisible ?? true,
    fillOpacity: cfg?.fillOpacity ?? 0.1,
  };
}

function macdToSpec(cfg: MACDConfig | undefined) {
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
function volumeToSpec(cfg: VolumeConfig | undefined) {
  return {
    enabled: cfg?.enabled ?? true,
    heightFrac: cfg?.height ?? -1,
    opacity: cfg?.opacity ?? -1,
    radiusPx: cfg?.radius ?? -1,
    upColor: (cfg?.upColor != null ? parseColor(cfg.upColor) : null) ?? 0,
    downColor: (cfg?.downColor != null ? parseColor(cfg.downColor) : null) ?? 0,
  };
}

// Price-line defaults: a soft red dotted rule with a dark translucent label,
// close in weight to the current-price indicator it sits beside.
const DEFAULT_PRICE_LINE_COLOR = 0xffef5350;
const DEFAULT_PRICE_LINE_BODY_BG = 0xd91c2128;
const DEFAULT_PRICE_LINE_HOVER_BOOST = 1.25;

const LINE_STYLES = { solid: 0, dotted: 1, dashed: 2 } as const;

// Mirrors VroomPriceLineFlags in packages/core/include/vroom/vroom_chart.h.
const PRICE_LINE_DRAGGABLE = 1 << 0;
const PRICE_LINE_CLOSABLE = 1 << 1;
const PRICE_LINE_AXIS_LABEL = 1 << 2;
const PRICE_LINE_EXTEND_LEFT = 1 << 3;

/** The price lines + their shared style, as the chart's props express them. */
export type PriceLinesProp = {
  lines: PriceLine[];
  style?: PriceLinesStyle;
  /**
   * Whether the host supplied a close handler. The close button is
   * callback-gated, so with nothing for it to do it isn't drawn at all.
   */
  hasCloseHandler: boolean;
};

function priceLinesToSpec(cfg: PriceLinesProp) {
  return {
    lines: cfg.lines.map((l) => ({
      price: l.price,
      color:
        (l.color != null ? parseColor(l.color) : null) ?? DEFAULT_PRICE_LINE_COLOR,
      width: l.width ?? 1,
      lineStyle: LINE_STYLES[l.lineStyle ?? 'dotted'],
      text: l.text ?? '',
      quantity: l.quantity ?? '',
      flags:
        (l.draggable ? PRICE_LINE_DRAGGABLE : 0) |
        (cfg.hasCloseHandler && l.closable !== false ? PRICE_LINE_CLOSABLE : 0) |
        (l.axisLabel !== false ? PRICE_LINE_AXIS_LABEL : 0) |
        (l.extendLeft !== false ? PRICE_LINE_EXTEND_LEFT : 0),
    })),
    bodyBg:
      (cfg.style?.bodyBackground != null ? parseColor(cfg.style.bodyBackground) : null) ??
      DEFAULT_PRICE_LINE_BODY_BG,
    fontSizePx: cfg.style?.fontSize ?? 0,
    lineLengthFrac: cfg.style?.inset ?? 0,
    align: cfg.style?.align === 'left' ? 0 : cfg.style?.align === 'center' ? 1 : 2,
    hoverBoost: cfg.style?.hoverBoost ?? DEFAULT_PRICE_LINE_HOVER_BOOST,
  };
}

// Cleared overlay: no lines (the style values are irrelevant, but the spec shape
// requires them).
const EMPTY_PRICE_LINES = priceLinesToSpec({ lines: [], hasCloseHandler: false });

let installed = false;
function ensureInstalled(): void {
  if (installed) return;
  const ok = NativeVroomChart.install();
  if (!ok) throw new Error('VroomChartModule.install() returned false');
  if (typeof globalThis.VroomChartJSI === 'undefined') {
    throw new Error('global.VroomChartJSI undefined after install()');
  }
  installed = true;
}

/** Progress + curve of the staggered volume-bar collapse. See setVolumeCollapse. */
export type VolumeCollapse = { t: number; easing: number };

/**
 * How a data swap should animate, plus where its frames go. The interval morph
 * has to be started from inside the data effect (it needs the pre-swap capture),
 * but it repaints at 60fps — far too often for React state — so the host passes
 * a sink that writes straight into the picture SharedValue.
 */
export type TransitionOptions = {
  /** Identity of the series; a change forces a full view reset. */
  seriesKey?: string;
  /** Duration of the interval morph in ms. 0 snaps. Default 300. */
  transitionMs?: number;
  /** Curve applied to the morph's progress. Default 'ease-in-out'. */
  transitionEasing?: TransitionEasing;
  /** OS reduced-motion preference: skips the capture and snaps. */
  reduceMotion?: boolean;
  /** Receives every morph frame. Without one, data swaps snap. */
  onFrame?: (picture: SkPicture) => void;
};

export type ChartCoreState = {
  handle: ChartHandle | null;
  /** Picture freshly rendered after the latest data/size/range push. */
  picture: SkPicture | null;
  /**
   * The last volume collapse handed to the core, or null before the first push.
   * VroomChart's animation loop owns this — it lives here only so the data effect
   * can restore it, since setVolume snaps the scalar (see below).
   */
  volumeCollapseRef: MutableRefObject<VolumeCollapse | null>;
};

// Owns a ChartHandle and produces an "initial" picture whenever data, size,
// or the externally-controlled visible range changes. Gesture-driven updates
// happen outside this hook by calling handle.pan(...) directly and assigning
// the result into a SharedValue.
export function useChartCore(
  candles: Candle[],
  size: { width: number; height: number; pxRatio?: number },
  visibleRange?: VisibleRange,
  defaultCandleWidth?: number,
  chartType?: ChartType,
  theme?: VroomTheme,
  rsi?: RSIConfig,
  macd?: MACDConfig,
  movingAverages?: MovingAverageOverlay[],
  vwap?: VWAPConfig,
  bollingerBands?: BollingerBandsConfig,
  volume?: VolumeConfig,
  priceLines?: PriceLinesProp,
  transition?: TransitionOptions,
): ChartCoreState {
  const handleRef = useRef<ChartHandle | null>(null);
  // Push setDefaultCandleWidth only once (first load): setCandles re-runs on
  // every data change, and the core setter re-frames when candles are present,
  // so re-pushing would snap the view away from the user's pan/zoom.
  const defaultWidthAppliedRef = useRef(false);
  const volumeCollapseRef = useRef<VolumeCollapse | null>(null);
  // What the core currently holds, for classifying the next data change. Keyed
  // by handle so a recreated core is treated as a fresh initial load.
  const prevDataRef = useRef<{
    handle: ChartHandle;
    candles: Candle[];
    seriesKey?: string;
  } | null>(null);
  const intervalMorphRaf = useRef<number | null>(null);
  const [picture, setPicture] = useState<SkPicture | null>(null);

  if (!handleRef.current && size.width > 0 && size.height > 0) {
    ensureInstalled();
    handleRef.current = globalThis.VroomChartJSI!.create();
  }

  // Animation config and frame sink in refs, refreshed each render, so changing
  // the duration, curve or callback identity doesn't re-run the data effect
  // below (which would re-push every candle).
  const animRef = useRef<{
    ms: number;
    easing: TransitionEasing | undefined;
    reduceMotion: boolean;
  }>({ ms: 300, easing: undefined, reduceMotion: false });
  animRef.current = {
    ms: Math.max(0, transition?.transitionMs ?? 300),
    easing: transition?.transitionEasing,
    reduceMotion: transition?.reduceMotion ?? false,
  };
  const onFrameRef = useRef(transition?.onFrame);
  onFrameRef.current = transition?.onFrame;
  const seriesKey = transition?.seriesKey;

  // Stop an in-flight interval morph and land the core on the new candles.
  const endIntervalMorph = useCallback(() => {
    if (intervalMorphRaf.current != null) {
      cancelAnimationFrame(intervalMorphRaf.current);
      intervalMorphRaf.current = null;
    }
    handleRef.current?.setIntervalMorph(1);
  }, []);

  // Runs the interval morph clock. The core holds the pre-swap geometry (see
  // beginIntervalMorph) and reshapes each candle slot toward its new counterpart.
  const startIntervalMorph = useCallback((h: ChartHandle) => {
    const { ms, easing } = animRef.current;
    const start = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - start) / ms);
      h.setIntervalMorph(p < 1 ? ease(easing, p) : 1);
      const pic = h.render();
      if (pic) onFrameRef.current?.(pic);
      intervalMorphRaf.current = p < 1 ? requestAnimationFrame(step) : null;
    };
    intervalMorphRaf.current = requestAnimationFrame(step);
  }, []);

  useEffect(() => {
    return () => {
      if (intervalMorphRaf.current != null) {
        cancelAnimationFrame(intervalMorphRaf.current);
        intervalMorphRaf.current = null;
      }
    };
  }, []);

  // When no visibleRange is provided, leave the range entirely to the C++
  // side (which defaults to a sensible recent window on first setCandles).
  // Only push setVisibleRange when the caller is actively controlling it,
  // so it doesn't clobber the default or fight gesture-driven pans.
  const explicit = visibleRange != null;
  const startMs = visibleRange?.startMs ?? 0;
  const endMs = visibleRange?.endMs ?? 0;

  // Stable deps so inline `theme={{...}}` / `rsi={{...}}` literals don't re-run
  // the effect every render — only when the actual values change.
  const themeKey = theme ? JSON.stringify(theme) : '';
  const rsiKey = rsi ? JSON.stringify(rsi) : '';
  const macdKey = macd ? JSON.stringify(macd) : '';
  const maKey = movingAverages ? JSON.stringify(movingAverages) : '';
  const vwapKey = vwap ? JSON.stringify(vwap) : '';
  const bollingerKey = bollingerBands ? JSON.stringify(bollingerBands) : '';
  const volumeKey = volume ? JSON.stringify(volume) : '';
  const priceLinesKey = priceLines ? JSON.stringify(priceLines) : '';

  useEffect(() => {
    const h = handleRef.current;
    if (!h) return;
    h.setSize(size.width, size.height, size.pxRatio ?? 1);
    // Drive the initial zoom from a target candle width. Pushed once, before the
    // first setCandles (while the core window is still 0/0), and only when the
    // caller isn't explicitly controlling the range.
    if (
      !defaultWidthAppliedRef.current &&
      !explicit &&
      defaultCandleWidth != null &&
      defaultCandleWidth > 0
    ) {
      h.setDefaultCandleWidth(defaultCandleWidth);
      defaultWidthAppliedRef.current = true;
    }
    // How the new candles relate to what the core holds decides what happens to
    // the viewport: a stream leaves it alone, a timeframe switch re-anchors and
    // morphs into it, a different asset resets it.
    let morphing = false;
    if (candles.length > 0) {
      const prev = prevDataRef.current;
      const freshHandle = prev == null || prev.handle !== h;
      if (freshHandle || prev.candles !== candles || prev.seriesKey !== seriesKey) {
        // A fresh core frames itself (its window starts at 0/0); an explicit
        // visibleRange prop overrides any auto behavior, so treat the change
        // like a stream and let the range application below win.
        const transitionKind: DataTransition = freshHandle
          ? 'initial'
          : explicit
            ? 'stream'
            : classifyTransition(prev.candles, candles, seriesKey !== prev.seriesKey);

        // Capture the outgoing view before setCandles re-infers the candle
        // period from the new data.
        let tfArgs: {
          oldWindow: VisibleRange;
          oldStepMs: number;
          oldLastMs: number;
        } | null = null;
        // The pre-swap candle envelope, used to scale-lock the y-axis below.
        let prevEnvelope: { low: number; high: number } | null = null;
        if (transitionKind === 'timeframe' && prev != null) {
          const oldWindow = h.getVisibleRange();
          const oldStepMs = inferStepMs(prev.candles);
          if (oldWindow.endMs > oldWindow.startMs && oldStepMs != null) {
            tfArgs = {
              oldWindow,
              oldStepMs,
              oldLastMs: prev.candles[prev.candles.length - 1].timeMs,
            };
          }
          prevEnvelope = h.getVisiblePriceEnvelope();
          // Capture the outgoing candle geometry, but only when it will actually
          // be animated so a disabled animation costs no snapshot. A switch
          // during a morph restarts from the data the core currently holds.
          morphing =
            animRef.current.ms > 0 &&
            !animRef.current.reduceMotion &&
            onFrameRef.current != null;
          if (morphing) {
            endIntervalMorph();
            h.beginIntervalMorph();
          }
        } else if (transitionKind === 'initial' || transitionKind === 'reset') {
          // Wholesale reframing — the slot pairing no longer holds, so land any
          // in-flight morph rather than reshaping into unrelated data.
          endIntervalMorph();
        }

        h.setCandles(packCandles(candles));

        if (transitionKind === 'timeframe') {
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
          if (morphing) startIntervalMorph(h);
        } else if (transitionKind === 'reset') {
          h.resetView();
        }
        prevDataRef.current = { handle: h, candles, seriesKey };
      }
    }
    if (explicit) {
      h.setVisibleRange(startMs, endMs);
    }
    // chartType / the candle↔line morph is driven separately (VroomChart owns the
    // per-frame animation loop so it can update the picture SharedValue directly).
    if (theme) {
      applyTheme(h, theme);
    }
    h.setRSI(rsiToSpec(rsi));
    h.setMACD(macdToSpec(macd));
    h.setOverlays((movingAverages ?? []).map(overlayToNumeric));
    h.setVWAP(vwapToSpec(vwap));
    h.setBollinger(bollingerToSpec(bollingerBands));
    h.setVolume(volumeToSpec(volume));
    // setVolume snaps the collapse scalar to its `enabled`, which would cut a
    // toggle animation short whenever this effect re-runs (a streaming candle, a
    // resize). Hand the in-flight value back; VroomChart's loop drives it from
    // there.
    const collapse = volumeCollapseRef.current;
    if (collapse) h.setVolumeCollapse(collapse.t, collapse.easing);
    h.setPriceLines(
      priceLines?.lines.length ? priceLinesToSpec(priceLines) : EMPTY_PRICE_LINES,
    );
    // TODO(rn-parity): mirror the web `liquidity` overlay here (setLiquidity +
    // the VroomBand structs in the JSI handle) — web-only for now.
    // A just-started morph is already pushing frames straight to the host sink;
    // this snapshot would land on top of them a frame or two later. The morph's
    // frame 0 is pixel-identical to what's on screen, so there's nothing to show
    // in the meantime anyway.
    if (!morphing) setPicture(h.render());
    // theme/rsi/macd/movingAverages/vwap/bollingerBands/volume/priceLines are
    // represented by their *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [candles, seriesKey, size.width, size.height, size.pxRatio, explicit, startMs, endMs, defaultCandleWidth, themeKey, rsiKey, macdKey, maKey, vwapKey, bollingerKey, volumeKey, priceLinesKey, startIntervalMorph, endIntervalMorph]);

  return { handle: handleRef.current, picture, volumeCollapseRef };
}
