// Runs the Opening Range Breakout assistant (@vroomchart/strategies) over the
// chart's candles, pushes its shapes to the core as annotations, and reports
// new events and each result to the host. The RN twin of
// packages/react/src/useOrb.ts.

import { useEffect, useRef, useState } from 'react';
import {
  inferStepMs,
  OrbEventTracker,
  orbToAnnotations,
  runOrb,
  type OrbAnnotations,
} from '@vroomchart/strategies';
import type { Candle, OrbConfig, OrbEvent, OrbResult, VroomColor } from '@vroomchart/types';
import type { ChartHandle } from './jsi.d';
import { parseColor } from './theme';

type AnnotationsSpec = Parameters<ChartHandle['setAnnotations']>[0];

export type OrbHookProps = {
  config?: OrbConfig;
  onOrbEvent?: (e: OrbEvent) => void;
  onOrbResult?: (r: OrbResult) => void;
};

const EMPTY_ANNOTATIONS: AnnotationsSpec = { segments: [], boxes: [], markers: [], labels: [] };
const DASH = { solid: 0, dotted: 1, dashed: 2 } as const;
const SHAPE = { triangleUp: 0, triangleDown: 1, cross: 2, square: 3, circle: 4 } as const;
const ALIGN = { start: 0, center: 1, end: 2 } as const;
// JSI casts to uint32 natively, so the packed color must already be unsigned.
const color = (c: VroomColor | undefined) => ((c != null ? parseColor(c) : null) ?? 0) >>> 0;

export function annotationsToSpec(a: OrbAnnotations): AnnotationsSpec {
  return {
    segments: a.segments.map((s) => ({
      aTime: s.a.timeMs, aPrice: s.a.price, bTime: s.b.timeMs, bPrice: s.b.price,
      color: color(s.color), width: s.width, dash: DASH[s.dash],
    })),
    boxes: a.boxes.map((b) => ({
      aTime: b.a.timeMs, aPrice: b.a.price, bTime: b.b.timeMs, bPrice: b.b.price,
      fill: color(b.fill), border: color(b.border), borderWidth: 1,
    })),
    markers: a.markers.map((m) => ({
      timeMs: m.timeMs, price: m.price, shape: SHAPE[m.shape], color: color(m.color),
      size: m.size, offset: m.offset,
    })),
    labels: a.labels.map((l) => ({
      timeMs: l.timeMs, price: l.price, text: l.text, color: color(l.color),
      background: color(l.background), align: ALIGN[l.align], offset: l.offset,
    })),
  };
}

/**
 * Re-runs on new candles, a config change, and when the forming bar's close
 * time passes. `repaint` renders a fresh picture after the annotations change.
 */
export function useOrb(
  handle: ChartHandle | null,
  candles: Candle[],
  seriesKey: string | undefined,
  orb: OrbHookProps | undefined,
  repaint: () => void,
): OrbResult | null {
  const config = orb?.config;
  const onEventRef = useRef(orb?.onOrbEvent);
  onEventRef.current = orb?.onOrbEvent;
  const onResultRef = useRef(orb?.onOrbResult);
  onResultRef.current = orb?.onOrbResult;
  const repaintRef = useRef(repaint);
  repaintRef.current = repaint;
  const trackerRef = useRef(new OrbEventTracker());
  const [result, setResult] = useState<OrbResult | null>(null);
  const [tick, setTick] = useState(0);
  const orbKey = config?.enabled ? JSON.stringify(config) : '';

  // The handle currently holding ORB annotations, so turning off clears once.
  const drawnRef = useRef<ChartHandle | null>(null);

  useEffect(() => {
    if (!handle) return;
    if (!config?.enabled || !candles.length) {
      if (drawnRef.current === handle) {
        drawnRef.current = null;
        handle.setAnnotations(EMPTY_ANNOTATIONS);
        trackerRef.current.reset();
        setResult(null);
        repaintRef.current();
      }
      return;
    }
    drawnRef.current = handle;
    const nowMs = Date.now();
    const stepMs = inferStepMs(candles);
    const res = runOrb(candles, config, { nowMs, stepMs });
    handle.setAnnotations(annotationsToSpec(orbToAnnotations(res, candles, config, stepMs)));
    repaintRef.current();
    const newest = candles[candles.length - 1].timeMs;
    for (const e of trackerRef.current.update(res, newest, `${seriesKey ?? ''}|${orbKey}`)) {
      onEventRef.current?.(e);
    }
    onResultRef.current?.(res);
    setResult(res);

    const closesAt = newest + stepMs;
    if (closesAt <= nowMs) return;
    const timer = setTimeout(() => setTick((t) => t + 1), closesAt - nowMs + 50);
    return () => clearTimeout(timer);
    // config is tracked via orbKey.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [handle, candles, orbKey, seriesKey, tick]);

  return result;
}
