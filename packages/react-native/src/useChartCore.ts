import { useCallback, useEffect, useRef, useState } from 'react';
import type { MutableRefObject } from 'react';

import NativeVroomChart from './NativeVroomChart';
import type { DataTransition } from './dataTransitions';
import {
  classifyStream,
  classifyTransition,
  inferStepMs,
  isPinnedToLatest,
  timeframeWindow,
} from './dataTransitions';
import { ease } from './easing';
import type { ChartFrame, ChartHandle } from './jsi.d';
import { packCandles } from './packCandles';
import { applyTheme, parseColor, FLOAT_LINE_TIP_PULSE } from './theme';
import type {
  BollingerBandsConfig,
  ATRConfig,
  Candle,
  ChartType,
  FairValueGapsConfig,
  IchimokuConfig,
  MACDConfig,
  MovingAverageOverlay,
  PriceLine,
  PriceLinesStyle,
  Footprint,
  FootprintsStyle,
  RSIConfig,
  TransitionEasing,
  IntervalTransition,
  StreamTransition,
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

// Mirrors vroom::atr::Smoothing order in packages/core/src/atr.h.
const ATR_SMOOTHINGS = ['rma', 'sma', 'ema'] as const;

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
    extremeFill: cfg?.extremeFill ?? true,
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

// Ichimoku defaults. Green and red do double duty: they color span A and kijun,
// and tint the cloud for whichever span is on top.
const DEFAULT_ICH_GREEN = 0xff26a69a;
const DEFAULT_ICH_RED = 0xffef5350;
const DEFAULT_ICH_BLUE = 0xff2962ff;
const DEFAULT_ICH_ORANGE = 0xffff6d00;
const DEFAULT_ICH_TEAL = 0xff00bcd4;

function ichimokuToSpec(cfg: IchimokuConfig | undefined) {
  const color = (v: string | number | undefined, fallback: number) =>
    (v != null ? parseColor(v) : null) ?? fallback;
  return {
    enabled: cfg?.enabled ?? false,
    tenkanPeriod: cfg?.tenkanPeriod ?? 9,
    kijunPeriod: cfg?.kijunPeriod ?? 26,
    senkouBPeriod: cfg?.senkouBPeriod ?? 52,
    displacement: cfg?.displacement ?? 26,
    tenkanColor: color(cfg?.tenkanColor, DEFAULT_ICH_BLUE),
    tenkanWidth: cfg?.tenkanWidth ?? 1,
    tenkanEnabled: cfg?.tenkanVisible ?? true,
    kijunColor: color(cfg?.kijunColor, DEFAULT_ICH_RED),
    kijunWidth: cfg?.kijunWidth ?? 1,
    kijunEnabled: cfg?.kijunVisible ?? true,
    senkouAColor: color(cfg?.senkouAColor, DEFAULT_ICH_GREEN),
    senkouAWidth: cfg?.senkouAWidth ?? 1,
    senkouAEnabled: cfg?.senkouAVisible ?? true,
    senkouBColor: color(cfg?.senkouBColor, DEFAULT_ICH_ORANGE),
    senkouBWidth: cfg?.senkouBWidth ?? 1,
    senkouBEnabled: cfg?.senkouBVisible ?? true,
    chikouColor: color(cfg?.chikouColor, DEFAULT_ICH_TEAL),
    chikouWidth: cfg?.chikouWidth ?? 1,
    chikouEnabled: cfg?.chikouVisible ?? true,
    cloudEnabled: cfg?.cloudVisible ?? true,
    bullishCloudColor: color(cfg?.bullishCloudColor, DEFAULT_ICH_GREEN),
    bearishCloudColor: color(cfg?.bearishCloudColor, DEFAULT_ICH_RED),
    cloudOpacity: cfg?.cloudOpacity ?? 0.15,
  };
}

// Fair Value Gap defaults. The border colors fall back to the fill color, so a
// config that only restyles the fill keeps its outline in the same hue.
const DEFAULT_FVG_GREEN = 0xff26a69a;
const DEFAULT_FVG_RED = 0xffef5350;
const FVG_FILL_TYPES = ['close', 'wick'] as const;
const FVG_BORDER_STYLES = ['solid', 'dotted', 'dashed'] as const;

function fvgToSpec(cfg: FairValueGapsConfig | undefined) {
  const color = (v: string | number | undefined, fallback: number) =>
    (v != null ? parseColor(v) : null) ?? fallback;
  const bullish = color(cfg?.bullishColor, DEFAULT_FVG_GREEN);
  const bearish = color(cfg?.bearishColor, DEFAULT_FVG_RED);
  return {
    enabled: cfg?.enabled ?? false,
    maxBarsBack: cfg?.maxBarsBack ?? 300,
    waitForClose: cfg?.waitForClose ?? false,
    fillType: Math.max(0, FVG_FILL_TYPES.indexOf(cfg?.fillType ?? 'close')),
    deleteAfterFill: cfg?.deleteAfterFill ?? true,
    extendBoxes: cfg?.extendBoxes ?? false,
    boxLength: cfg?.boxLength ?? 20,
    bullishColor: bullish,
    bearishColor: bearish,
    opacity: cfg?.opacity ?? 0.15,
    borderEnabled: cfg?.borderVisible ?? true,
    borderStyle: Math.max(
      0,
      FVG_BORDER_STYLES.indexOf(cfg?.borderStyle ?? 'solid'),
    ),
    borderWidth: cfg?.borderWidth ?? 1,
    bullishBorderColor: color(cfg?.bullishBorderColor, bullish),
    bearishBorderColor: color(cfg?.bearishBorderColor, bearish),
    labelsEnabled: cfg?.showLabels ?? true,
    label: cfg?.label ?? 'FVG',
    labelDistance: cfg?.labelDistance ?? 10,
    // Alpha 0 is the core's "inherit the border color" sentinel.
    labelColor: color(cfg?.labelColor, 0),
    labelFontSize: cfg?.labelFontSize ?? 0,
    showInverse: cfg?.showInverse ?? false,
    inverseBullishColor: color(cfg?.inverseBullishColor, bullish),
    inverseBearishColor: color(cfg?.inverseBearishColor, bearish),
    inverseLabel: cfg?.inverseLabel ?? 'iFVG',
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

function atrToSpec(cfg: ATRConfig | undefined) {
  return {
    enabled: cfg?.enabled ?? false,
    period: cfg?.period ?? 14,
    smoothing: Math.max(0, ATR_SMOOTHINGS.indexOf(cfg?.smoothing ?? 'rma')),
    lineColor: inheritColor(cfg?.lineColor),
    lineWidth: cfg?.lineWidth ?? -1,
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

// Footprints share the price lines' hover weight so the two widgets light up
// alike. Zeroed geometry defers to the core's own defaults.
const DEFAULT_FOOTPRINT_HOVER_BOOST = 1.25;

// Mirrors VroomFootprintSide in packages/core/include/vroom/vroom_chart.h.
const FOOTPRINT_BUY = 0;
const FOOTPRINT_SELL = 1;

// A time no real series can contain (~273,000 BCE), still comfortably inside
// int64. Parks a malformed footprint where the core will never bucket it.
const UNBUCKETABLE_MS = -8.64e15;

/** The footprints + their shared style, as the chart's props express them. */
export type FootprintsProp = {
  prints: Footprint[];
  style?: FootprintsStyle;
};

function footprintsToSpec(cfg: FootprintsProp) {
  return {
    // Index alignment is load-bearing: the core reports hits as indices into this
    // array and the gesture layer maps them straight back to the consumer's
    // `footprints`. So a non-finite time — which can't be bucketed and would
    // reach the native side as a garbage int64 — is neutralized *in place* rather
    // than filtered out, which would shift every index after it onto the wrong
    // trade.
    prints: cfg.prints.map((f) => ({
      timeMs: Number.isFinite(f.timeMs) ? f.timeMs : UNBUCKETABLE_MS,
      side: f.side === 'sell' ? FOOTPRINT_SELL : FOOTPRINT_BUY,
    })),
    radiusPx: cfg.style?.radius ?? 0,
    gapPx: cfg.style?.gap ?? 0,
    marginPx: cfg.style?.margin ?? 0,
    hoverBoost: cfg.style?.hoverBoost ?? DEFAULT_FOOTPRINT_HOVER_BOOST,
  };
}

const EMPTY_FOOTPRINTS = footprintsToSpec({ prints: [] });

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
  /** `'transform'` (default) slot-lerps; `'fade'` fades out then in. */
  intervalTransition?: IntervalTransition;
  /** `'transform'` eases live updates; `'none'` (default) snaps them. */
  streamTransition?: StreamTransition;
  /** Duration of the stream animation in ms. 0 snaps. Default 150. */
  streamTransitionMs?: number;
  /** OS reduced-motion preference: skips the capture and snaps. */
  reduceMotion?: boolean;
  /** Receives every morph frame. Without one, data swaps snap. */
  onFrame?: (picture: ChartFrame) => void;
};

export type ChartCoreState = {
  handle: ChartHandle | null;
  /** Picture freshly rendered after the latest data/size/range push. */
  picture: ChartFrame | null;
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
  atr?: ATRConfig,
  movingAverages?: MovingAverageOverlay[],
  vwap?: VWAPConfig,
  bollingerBands?: BollingerBandsConfig,
  ichimoku?: IchimokuConfig,
  fairValueGaps?: FairValueGapsConfig,
  volume?: VolumeConfig,
  priceLines?: PriceLinesProp,
  footprints?: FootprintsProp,
  transition?: TransitionOptions,
  // Trails the config params because it's data state, not configuration: it
  // pairs with `candles` above (see showLoadingLine below).
  loading?: boolean,
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
  const streamRaf = useRef<number | null>(null);
  // Where an in-flight stream shift is headed, so cancelling it can land there
  // rather than stranding the view mid-slide.
  const streamWindowRef = useRef<VisibleRange | null>(null);
  // Whether that loop is the one driving the morph scalar, so settling it never
  // cuts short a timeframe switch that happens to overlap.
  const streamMorphRef = useRef(false);
  const [picture, setPicture] = useState<ChartFrame | null>(null);

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
    interval: IntervalTransition;
    stream: StreamTransition;
    streamMs: number;
  }>({
    ms: 300,
    easing: undefined,
    reduceMotion: false,
    interval: 'transform',
    stream: 'none',
    streamMs: 150,
  });
  animRef.current = {
    ms: Math.max(0, transition?.transitionMs ?? 300),
    easing: transition?.transitionEasing,
    reduceMotion: transition?.reduceMotion ?? false,
    interval: transition?.intervalTransition === 'fade' ? 'fade' : 'transform',
    stream: transition?.streamTransition === 'transform' ? 'transform' : 'none',
    // Shorter than transitionMs by default: ticks can land faster than a 300ms
    // curve, and every one that does interrupts the last.
    streamMs: Math.max(0, transition?.streamTransitionMs ?? 150),
  };
  const onFrameRef = useRef(transition?.onFrame);
  onFrameRef.current = transition?.onFrame;
  const seriesKey = transition?.seriesKey;

  // Set only while the loading hand-off is in its *first* stage, which is the
  // one an interruption can't simply land: stage one leaves `loading` set in
  // the core, and only stage two releases it.
  const loadingHandoffRef = useRef(false);

  // Stop an in-flight interval morph and land the core on the new candles.
  const endIntervalMorph = useCallback(() => {
    if (intervalMorphRaf.current != null) {
      cancelAnimationFrame(intervalMorphRaf.current);
      intervalMorphRaf.current = null;
    }
    const h = handleRef.current;
    // Walk a half-finished hand-off through the rest of its stages rather than
    // just stopping the clock, or the core would be left drawing the loading
    // line over the data it was supposed to hand off to.
    if (loadingHandoffRef.current) {
      loadingHandoffRef.current = false;
      h?.setLoadingMorph(1);
      h?.beginLoadingReveal();
    }
    h?.setIntervalMorph(1);
  }, []);

  // Runs the interval morph clock. The core holds the pre-swap geometry (see
  // beginIntervalMorph) and reshapes each candle slot toward its new counterpart.
  // `durationMs` overrides transitionMs for the loading hand-off, which splits
  // it across two stages.
  const startIntervalMorph = useCallback((h: ChartHandle, durationMs?: number) => {
    const { ms, easing } = animRef.current;
    const dur = durationMs ?? ms;
    const start = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - start) / dur);
      h.setIntervalMorph(p < 1 ? ease(easing, p) : 1);
      const pic = h.render();
      if (pic) onFrameRef.current?.(pic);
      intervalMorphRaf.current = p < 1 ? requestAnimationFrame(step) : null;
    };
    intervalMorphRaf.current = requestAnimationFrame(step);
  }, []);

  // Hands the loading line over to the data that just landed, in two stages:
  // the line reshapes into the series' silhouette, then the candles grow out of
  // it while it fades.
  //
  // Sequential rather than overlapped — the shape has to read as the data
  // before the bars start emerging from it — so the two split transitionMs and
  // the whole hand-off costs what any other transition costs.
  const startLoadingHandoff = useCallback(
    (h: ChartHandle) => {
      const { ms, easing } = animRef.current;
      const half = ms / 2;
      loadingHandoffRef.current = true;
      h.beginLoadingMorph();
      const start = performance.now();
      const step = (now: number) => {
        const p = Math.min(1, (now - start) / half);
        h.setLoadingMorph(p < 1 ? ease(easing, p) : 1);
        const pic = h.render();
        if (pic) onFrameRef.current?.(pic);
        if (p < 1) {
          intervalMorphRaf.current = requestAnimationFrame(step);
          return;
        }
        intervalMorphRaf.current = null;
        // Past here an interruption is an ordinary interval morph again: the
        // core has left the loading state, so landing the clock is enough.
        loadingHandoffRef.current = false;
        // Stage two rides the interval-morph clock, which also carries the
        // line's fade-out — so the bars' growth and the line's exit finish
        // together instead of one outlasting the other.
        h.beginLoadingReveal();
        startIntervalMorph(h, half);
      };
      intervalMorphRaf.current = requestAnimationFrame(step);
    },
    [startIntervalMorph],
  );

  // Stops an in-flight stream animation and puts the chart somewhere coherent.
  //
  // A pending window shift always lands on its target: abandoned mid-slide it
  // would strand the view between two bars, half a candle off the grid.
  //
  // `keepMorph` is for a tick restarting on top of one already running —
  // beginStreamMorph blends out of the geometry currently on screen, so landing
  // that geometry first would throw away the very thing it resumes from.
  const settleStream = useCallback((keepMorph = false) => {
    if (streamRaf.current != null) {
      cancelAnimationFrame(streamRaf.current);
      streamRaf.current = null;
    }
    const h = handleRef.current;
    const target = streamWindowRef.current;
    streamWindowRef.current = null;
    if (target) h?.setVisibleRange(target.startMs, target.endMs);
    if (streamMorphRef.current && !keepMorph) {
      streamMorphRef.current = false;
      h?.setIntervalMorph(1);
    }
  }, []);

  // Runs the clock for a live update. One loop drives both halves so they land
  // on the same frame.
  //
  // `window` is null for a plain tick; for an append it is where the view has to
  // end up. The slide is measured from wherever the window is *now*, so a shift
  // interrupting another continues from the current position instead of
  // snapping back to the start of the last one.
  const startStreamAnim = useCallback(
    (h: ChartHandle, morphing: boolean, window: VisibleRange | null) => {
      const { streamMs, easing } = animRef.current;
      let from = window ? h.getVisibleRange() : null;
      // What the previous frame left the window at. Anything else — a pan, a
      // pinch — lands somewhere different, which is how the slide notices it is
      // no longer the only thing moving the view and gets out of the way.
      // Cheaper than teaching every gesture to cancel it, and it can't miss one.
      let applied: VisibleRange | null = null;
      streamWindowRef.current = window;
      streamMorphRef.current = morphing;
      const start = performance.now();
      const step = (now: number) => {
        if (from && applied) {
          const now_w = h.getVisibleRange();
          if (now_w.startMs !== applied.startMs || now_w.endMs !== applied.endMs) {
            from = null;
            streamWindowRef.current = null;
          }
        }
        const p = Math.min(1, (now - start) / streamMs);
        const e = p < 1 ? ease(easing, p) : 1;
        if (morphing) h.setIntervalMorph(e);
        if (from && window) {
          applied = {
            startMs: Math.round(from.startMs + (window.startMs - from.startMs) * e),
            endMs: Math.round(from.endMs + (window.endMs - from.endMs) * e),
          };
          h.setVisibleRange(applied.startMs, applied.endMs);
        }
        const pic = h.render();
        if (pic) onFrameRef.current?.(pic);
        if (p < 1) {
          streamRaf.current = requestAnimationFrame(step);
        } else {
          streamRaf.current = null;
          streamWindowRef.current = null;
          streamMorphRef.current = false;
        }
      };
      streamRaf.current = requestAnimationFrame(step);
    },
    [],
  );

  useEffect(() => {
    return () => {
      if (intervalMorphRaf.current != null) {
        cancelAnimationFrame(intervalMorphRaf.current);
        intervalMorphRaf.current = null;
      }
      if (streamRaf.current != null) {
        cancelAnimationFrame(streamRaf.current);
        streamRaf.current = null;
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

  // The core trusts `setLoading` outright, so the "and no data yet" half of the
  // condition is decided here. Both halves matter: without `loading` a chart
  // that legitimately has no bars would wave a placeholder forever, and without
  // the emptiness check a background refresh of a loaded series would blank the
  // chart the user is already reading.
  const showLoadingLine = loading === true && candles.length === 0;
  // Tracks whether the *core* is currently showing the line, which is what
  // decides if the next data push is a hand-off. Distinct from `showLoadingLine`:
  // that is this render's intent, this is what's on screen.
  const lineUpRef = useRef(false);

  // Stable deps so inline `theme={{...}}` / `rsi={{...}}` literals don't re-run
  // the effect every render — only when the actual values change.
  const themeKey = theme ? JSON.stringify(theme) : '';
  const rsiKey = rsi ? JSON.stringify(rsi) : '';
  const macdKey = macd ? JSON.stringify(macd) : '';
  const atrKey = atr ? JSON.stringify(atr) : '';
  const maKey = movingAverages ? JSON.stringify(movingAverages) : '';
  const vwapKey = vwap ? JSON.stringify(vwap) : '';
  const bollingerKey = bollingerBands ? JSON.stringify(bollingerBands) : '';
  const ichimokuKey = ichimoku ? JSON.stringify(ichimoku) : '';
  const fvgKey = fairValueGaps ? JSON.stringify(fairValueGaps) : '';
  const volumeKey = volume ? JSON.stringify(volume) : '';
  const priceLinesKey = priceLines ? JSON.stringify(priceLines) : '';
  const footprintsKey = footprints ? JSON.stringify(footprints) : '';

  useEffect(() => {
    const h = handleRef.current;
    if (!h) return;
    h.setSize(size.width, size.height, size.pxRatio ?? 1);
    // Ahead of setCandles, like setDefaultCandleWidth below: the default framing
    // runs inside setCandles and reserves room past the newest candle for
    // Ichimoku's leading spans, so it has to already know they're coming.
    h.setIchimoku(ichimokuToSpec(ichimoku));
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

    if (showLoadingLine) {
      // Clear the core's buffer, which the `candles.length > 0` gate below
      // otherwise never does: pushing an empty array is treated as "hold the
      // last frame" everywhere else, so a chart switching assets would still be
      // holding the previous one's bars underneath the line — and would
      // classify the incoming series as a timeframe switch rather than a fresh
      // load. Scoped to the loading case so that hold-the-last-frame behavior
      // is untouched for every other empty push.
      if (!lineUpRef.current) {
        endIntervalMorph();
        settleStream();
        h.setCandles(packCandles([]));
        prevDataRef.current = null;
      }
      h.setLoading(true, !animRef.current.reduceMotion);
      lineUpRef.current = true;
    } else if (lineUpRef.current && candles.length === 0) {
      // Loading resolved to nothing — an empty result, or an error the consumer
      // handled. There's no geometry to morph into, so drop the line rather
      // than leaving it waving at data that isn't coming.
      h.setLoading(false, true);
      lineUpRef.current = false;
    }

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
        // Set when this push is the loading line's hand-off to real data.
        let handOff = false;
        // Set for an animated live update: whether the last bar reshapes, and
        // the window an appended bar should pull the view to.
        let stream: { morph: boolean; window: VisibleRange | null } | null = null;
        if (transitionKind === 'stream' && prev != null && !explicit) {
          const { stream: mode, streamMs, reduceMotion } = animRef.current;
          const stepMs = inferStepMs(candles);
          if (
            mode === 'transform' &&
            streamMs > 0 &&
            stepMs != null &&
            !reduceMotion &&
            onFrameRef.current != null
          ) {
            const lastMs = candles[candles.length - 1].timeMs;
            const prevLastMs = prev.candles[prev.candles.length - 1].timeMs;
            if (classifyStream(prev.candles, candles) === 'append') {
              // Pull the window along by exactly what the data advanced, so the
              // series translates a whole slot and the newest bar holds its
              // place on screen. Only for a view still following the newest bar
              // — someone reading history keeps their window.
              //
              // No capture here: slots pair from the right edge, so the new bar
              // would take the previous one's geometry and drag every candle
              // onto its neighbour. Translating the window moves them by their
              // own timestamps instead.
              const w = h.getVisibleRange();
              const prevStepMs = inferStepMs(prev.candles) ?? stepMs;
              if (isPinnedToLatest(w, prevLastMs, prevStepMs)) {
                const by = lastMs - prevLastMs;
                stream = {
                  morph: false,
                  window: { startMs: w.startMs + by, endMs: w.endMs + by },
                };
              }
            } else {
              stream = { morph: true, window: null };
            }
          }
          if (stream?.morph) {
            // Keep the geometry on screen for beginStreamMorph to resume from:
            // at any real tick rate most ticks interrupt the previous one, and
            // that continuity is what keeps the bar from stuttering.
            settleStream(true);
            h.beginStreamMorph();
          } else {
            // An append has no use for a capture — it would pair the new bar
            // with the old one's geometry and drag the whole series along.
            settleStream();
          }
        } else if (transitionKind === 'stream') {
          settleStream();
        }
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
            h.beginIntervalMorph(animRef.current.interval);
          }
        } else if (transitionKind === 'initial' || transitionKind === 'reset') {
          // Wholesale reframing — the slot pairing no longer holds, so land any
          // in-flight morph rather than reshaping into unrelated data.
          endIntervalMorph();
        }

        // The loading line's data has landed, so it hands over to the series
        // instead of the chart cutting to it. Always classified 'initial' (the
        // loading branch above cleared prevDataRef), so this runs after that
        // branch's endIntervalMorph.
        if (lineUpRef.current) {
          lineUpRef.current = false;
          handOff =
            animRef.current.ms > 0 &&
            !animRef.current.reduceMotion &&
            onFrameRef.current != null;
          // Snap path only. The hand-off itself starts after setCandles and the
          // framing below: both its stages aim at where the candles will
          // actually sit, so neither can be set up until they're there.
          if (!handOff) h.setLoading(false, true);
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
        } else if (stream) {
          // After setCandles, so the capture (and the window it slides from) is
          // measured against the data the animation is heading toward.
          startStreamAnim(h, stream.morph, stream.window);
        } else if (transitionKind === 'reset') {
          h.resetView();
        }
        // After setCandles and the framing above, so the line aims at — and the
        // candles grow from — the geometry each bar will actually occupy.
        if (handOff) startLoadingHandoff(h);
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
    // The tip dot stays, only its animation drops — the same bargain the
    // candle↔line morph strikes when it keeps the crossfade but skips the
    // collapse. Also stops the pulse from pinning a RAF loop for a user who
    // asked for less motion.
    if (animRef.current.reduceMotion) {
      h.setFloat(FLOAT_LINE_TIP_PULSE, 0);
    }
    h.setRSI(rsiToSpec(rsi));
    h.setMACD(macdToSpec(macd));
    h.setATR(atrToSpec(atr));
    h.setOverlays((movingAverages ?? []).map(overlayToNumeric));
    h.setVWAP(vwapToSpec(vwap));
    h.setBollinger(bollingerToSpec(bollingerBands));
    h.setFairValueGaps(fvgToSpec(fairValueGaps));
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
    h.setFootprints(
      footprints?.prints.length ? footprintsToSpec(footprints) : EMPTY_FOOTPRINTS,
    );
    // TODO(rn-parity): mirror the web `liquidity` overlay here (setLiquidity +
    // the VroomBand structs in the JSI handle) — web-only for now.
    // A just-started morph is already pushing frames straight to the host sink;
    // this snapshot would land on top of them a frame or two later. The morph's
    // frame 0 is pixel-identical to what's on screen, so there's nothing to show
    // in the meantime anyway.
    if (!morphing) setPicture(h.render());
    // theme/rsi/macd/atr/movingAverages/vwap/bollingerBands/ichimoku/
    // fairValueGaps/volume/priceLines/footprints are represented by their *Key
    // deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [candles, showLoadingLine, seriesKey, size.width, size.height, size.pxRatio, explicit, startMs, endMs, defaultCandleWidth, themeKey, rsiKey, macdKey, atrKey, maKey, vwapKey, bollingerKey, ichimokuKey, fvgKey, volumeKey, priceLinesKey, footprintsKey, startIntervalMorph, startLoadingHandoff, endIntervalMorph, startStreamAnim, settleStream]);

  return { handle: handleRef.current, picture, volumeCollapseRef };
}
