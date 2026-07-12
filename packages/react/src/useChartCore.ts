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
  type LoadVroomOptions,
  type OverlaySpec,
  type DrawingSpec,
  type LiquiditySpec,
  type VroomChartHandle,
} from '@vroomchart/core-wasm';
import type { VroomChartCoreProps } from '@vroomchart/types';
import {
  classifyTransition,
  inferStepMs,
  timeframeWindow,
  type DataTransition,
} from './dataTransitions';

// Mirrors vroom::ma::Source order (packages/core/src/ma.h).
const MA_SOURCES = ['close', 'open', 'high', 'low', 'hl2', 'hlc3', 'ohlc4'] as const;

function overlayToNumeric(
  o: NonNullable<VroomChartCoreProps['movingAverages']>[number],
): OverlaySpec {
  const srcIdx = o.source ? MA_SOURCES.indexOf(o.source) : 0;
  return {
    kind: o.kind === 'ema' ? 1 : 0,
    period: o.length,
    source: srcIdx < 0 ? 0 : srcIdx,
    color: (o.color != null ? parseColor(o.color) : null) ?? 0xff2962ff,
    width: o.width ?? 1.5,
  };
}

function drawingToSpec(
  d: NonNullable<VroomChartCoreProps['drawings']>[number],
): DrawingSpec {
  return {
    aTime: d.points[0].timeMs,
    aPrice: d.points[0].price,
    bTime: d.points[1].timeMs,
    bPrice: d.points[1].price,
    color: (d.color != null ? parseColor(d.color) : null) ?? 0xff2962ff,
    width: d.width ?? 2,
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

export type UseChartCore = {
  containerRef: React.RefObject<HTMLDivElement | null>;
  canvasRef: React.RefObject<HTMLCanvasElement | null>;
  handleRef: React.RefObject<VroomChartHandle | null>;
  /** Schedules a rAF-batched repaint (and keeps ticking while animating). */
  scheduleRender: () => void;
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
    theme,
    rsi,
    macd,
    movingAverages,
    vwap,
    drawings,
    liquidity,
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
  const [ready, setReady] = useState(false);
  const [measured, setMeasured] = useState({ width: 0, height: 0 });

  // Captured at mount: which core to load (stub vs Skia-WASM). Changing it after
  // mount has no effect — the core is created once and shared process-wide.
  const loadOptsRef = useRef(loadOpts);
  loadOptsRef.current = loadOpts;

  const width = widthProp ?? measured.width;
  const height = heightProp ?? measured.height;

  const scheduleRender = useCallback(() => {
    if (rafRef.current != null) return;
    rafRef.current = requestAnimationFrame(() => {
      rafRef.current = null;
      const h = handleRef.current;
      if (!h) return;
      h.present();
      if (h.isAnimating()) scheduleRender();
    });
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
  const drawingsKey = drawings ? JSON.stringify(drawings) : '';
  const liquidityKey = liquidity ? JSON.stringify(liquidity) : '';
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
        if (transition === 'timeframe' && prev != null) {
          const oldWindow = h.getVisibleRange();
          const oldStepMs = inferStepMs(prev.candles);
          if (oldWindow.endMs > oldWindow.startMs && oldStepMs != null) {
            tfArgs = { oldWindow, oldStepMs, oldLastMs: prev.candles[prev.candles.length - 1].timeMs };
          }
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
          h.resetPriceScale();
        } else if (transition === 'reset') {
          h.resetView();
        }
        prevDataRef.current = { handle: h, candles, seriesKey };
      }
    }
    if (explicit) h.setVisibleRange(startMs, endMs);
    if (theme) applyTheme(h, theme);
    h.setRSI(
      rsi?.enabled ?? false,
      rsi?.period ?? 14,
      rsi?.upperBand ?? 70,
      rsi?.lowerBand ?? 30,
      rsi?.maEnabled ?? true,
      rsi?.maPeriod ?? 14,
    );
    h.setMACD(macd?.enabled ?? false, macd?.fast ?? 12, macd?.slow ?? 26, macd?.signal ?? 9);
    h.setOverlays((movingAverages ?? []).map(overlayToNumeric));
    h.setVWAP(
      vwap?.enabled ?? false,
      vwap?.resetMinutes ?? 0,
      (vwap?.color != null ? parseColor(vwap.color) : null) ?? 0xff00bcd4,
      vwap?.width ?? 1.5,
    );
    h.setDrawings((drawings ?? []).map(drawingToSpec));
    h.setLiquidity(
      liquidity?.bands?.length ? liquidityToSpec(liquidity) : EMPTY_LIQUIDITY,
    );
    scheduleRender();
    // theme/rsi/macd/movingAverages/vwap/drawings/liquidity tracked via *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ready, width, height, candles, seriesKey, explicit, startMs, endMs, themeKey, rsiKey, macdKey, maKey, vwapKey, drawingsKey, liquidityKey, scheduleRender]);

  return { containerRef, canvasRef, handleRef, scheduleRender, size: { width, height } };
}
