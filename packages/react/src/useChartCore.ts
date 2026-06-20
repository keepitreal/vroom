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
  type OverlaySpec,
  type VroomChartHandle,
} from '@vroom/core-wasm';
import type { VroomChartCoreProps } from '@vroom/types';

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

export type UseChartCore = {
  containerRef: React.RefObject<HTMLDivElement | null>;
  canvasRef: React.RefObject<HTMLCanvasElement | null>;
  handleRef: React.RefObject<VroomChartHandle | null>;
  /** Schedules a rAF-batched repaint (and keeps ticking while animating). */
  scheduleRender: () => void;
  /** Measured CSS size of the container (0 until first layout). */
  size: { width: number; height: number };
};

export function useChartCore(props: VroomChartCoreProps): UseChartCore {
  const {
    candles,
    width: widthProp,
    height: heightProp,
    visibleRange,
    theme,
    rsi,
    macd,
    movingAverages,
    vwap,
  } = props;

  const containerRef = useRef<HTMLDivElement | null>(null);
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const handleRef = useRef<VroomChartHandle | null>(null);
  const rafRef = useRef<number | null>(null);
  const [ready, setReady] = useState(false);
  const [measured, setMeasured] = useState({ width: 0, height: 0 });

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
    loadVroom().then((mod) => {
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
  const explicit = visibleRange != null;
  const startMs = visibleRange?.startMs ?? 0;
  const endMs = visibleRange?.endMs ?? 0;

  // Push everything into the core whenever data/size/config changes, then paint.
  useEffect(() => {
    const h = handleRef.current;
    if (!h || width <= 0 || height <= 0) return;
    const dpr = typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1;
    h.setSize(width, height, dpr);
    if (candles.length > 0) h.setCandles(packCandles(candles));
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
    scheduleRender();
    // theme/rsi/macd/movingAverages/vwap tracked via their *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ready, width, height, candles, explicit, startMs, endMs, themeKey, rsiKey, macdKey, maKey, vwapKey, scheduleRender]);

  return { containerRef, canvasRef, handleRef, scheduleRender, size: { width, height } };
}
