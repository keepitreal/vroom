'use client';

import { useEffect, useRef, useState } from 'react';
import { type Candle, type VisibleRange, type VroomChartProps } from '@vroomchart/react';
import { VroomChart } from './ClientChart';
import { STEP_MS } from './market';
import type { Frame } from './scenes';

type Props = {
  candles: Candle[];
  frame: Frame;
  /** Breathe the window in and out, as if someone were pinching. */
  pinching: boolean;
  reduceMotion: boolean;
  chartProps: Partial<VroomChartProps>;
};

export const toRange = (lastMs: number, bars: number, pad: number): VisibleRange => {
  const endMs = lastMs + pad * STEP_MS;
  return { startMs: Math.round(endMs - bars * STEP_MS), endMs: Math.round(endMs) };
};

/**
 * The fixed chart behind the page. The page owns its window and eases it toward
 * each section's frame; visitors play with the separate full-screen player.
 */
export function ChartLayer({ candles, frame, pinching, reduceMotion, chartProps }: Props) {
  const lastMs = candles[candles.length - 1].timeMs;
  const [range, setRange] = useState<VisibleRange>(() => toRange(lastMs, frame.bars, frame.pad));
  const live = useRef({ bars: frame.bars, pad: frame.pad });
  const target = useRef({ frame, pinching, lastMs, reduceMotion });
  target.current = { frame, pinching, lastMs, reduceMotion };

  useEffect(() => {
    let raf = 0;
    const t0 = performance.now();
    let prev = '';
    const tick = (now: number) => {
      const { frame: f, pinching: p, lastMs: last, reduceMotion: still } = target.current;
      const cur = live.current;
      const k = still ? 1 : 0.08;
      cur.bars += (f.bars - cur.bars) * k;
      cur.pad += (f.pad - cur.pad) * k;
      let bars = cur.bars;
      let pad = cur.pad;
      if (p && !still) {
        const s = (now - t0) / 1000;
        bars *= 1 + 0.5 * Math.sin(s * 0.9);
        pad -= 30 * (0.5 - 0.5 * Math.cos(s * 0.45));
      }
      const next = toRange(last, bars, pad);
      const key = `${next.startMs}:${next.endMs}`;
      if (key !== prev) {
        prev = key;
        setRange(next);
      }
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, []);

  return (
    <div className="chart-layer" aria-hidden>
      <VroomChart
        candles={candles}
        seriesKey="BTC-PERP"
        reduceMotion={reduceMotion || undefined}
        transitionMs={450}
        streamTransition="transform"
        autoscaleY
        {...chartProps}
        visibleRange={range}
      />
    </div>
  );
}
