// Runs the Opening Range Breakout assistant (@vroomchart/strategies) over the
// chart's candles, pushes its shapes to the core as annotations, and reports
// new events and each result to the host. The strategy itself is pure; this
// only feeds it the clock, re-runs it when the forming bar closes, and dedupes
// events across re-runs.

import { useEffect, useRef, useState } from 'react';
import { parseColor, type AnnotationsSpec, type VroomChartHandle } from '@vroomchart/core-wasm';
import {
  inferStepMs,
  OrbEventTracker,
  orbToAnnotations,
  runOrb,
  type OrbAnnotations,
} from '@vroomchart/strategies';
import type { OrbResult, VroomChartCoreProps, VroomColor } from '@vroomchart/types';

export const EMPTY_ANNOTATIONS: AnnotationsSpec = { segments: [], boxes: [], markers: [], labels: [] };

const DASH = { solid: 0, dotted: 1, dashed: 2 } as const;
const SHAPE = { triangleUp: 0, triangleDown: 1, cross: 2, square: 3, circle: 4 } as const;
const ALIGN = { start: 0, center: 1, end: 2 } as const;
const color = (c: VroomColor | undefined) => (c != null ? parseColor(c) : null) ?? 0;

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

export function useOrb(
  handle: VroomChartHandle | null,
  props: Pick<VroomChartCoreProps, 'candles' | 'orb' | 'onOrbEvent' | 'onOrbResult' | 'seriesKey'>,
  scheduleRender: () => void,
): OrbResult | null {
  const { candles, orb, seriesKey } = props;
  const onEventRef = useRef(props.onOrbEvent);
  onEventRef.current = props.onOrbEvent;
  const onResultRef = useRef(props.onOrbResult);
  onResultRef.current = props.onOrbResult;
  const trackerRef = useRef(new OrbEventTracker());
  const [result, setResult] = useState<OrbResult | null>(null);
  // Bumped when the forming bar's close time passes, to re-run without new data.
  const [tick, setTick] = useState(0);
  const orbKey = orb?.enabled ? JSON.stringify(orb) : '';

  // The handle currently holding ORB annotations, so turning off clears once.
  const drawnRef = useRef<VroomChartHandle | null>(null);

  useEffect(() => {
    if (!handle) return;
    if (!orb?.enabled || !candles?.length) {
      if (drawnRef.current === handle) {
        drawnRef.current = null;
        handle.setAnnotations(EMPTY_ANNOTATIONS);
        trackerRef.current.reset();
        setResult(null);
        scheduleRender();
      }
      return;
    }
    drawnRef.current = handle;
    const nowMs = Date.now();
    const stepMs = inferStepMs(candles);
    const res = runOrb(candles, orb, { nowMs, stepMs });
    handle.setAnnotations(annotationsToSpec(orbToAnnotations(res, candles, orb, stepMs)));
    scheduleRender();
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
    // orb is tracked via orbKey.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [handle, candles, orbKey, seriesKey, tick, scheduleRender]);

  return result;
}
