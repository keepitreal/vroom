import { describe, expect, it } from 'vitest';
import type { Candle } from '@vroomchart/types';
import { classifyTransition, inferStepMs, timeframeWindow } from './dataTransitions';

const MINUTE = 60_000;

function series(opts: {
  count: number;
  stepMs: number;
  endMs: number;
  price: number;
}): Candle[] {
  const { count, stepMs, endMs, price } = opts;
  const out: Candle[] = [];
  for (let i = 0; i < count; i++) {
    const timeMs = endMs - (count - 1 - i) * stepMs;
    // Deterministic mild wobble around `price` so bounds are non-degenerate.
    const open = price * (1 + 0.001 * Math.sin(i));
    const close = price * (1 + 0.001 * Math.cos(i));
    out.push({
      timeMs,
      open,
      close,
      high: Math.max(open, close) * 1.001,
      low: Math.min(open, close) * 0.999,
      volume: 100,
    });
  }
  return out;
}

const NOW = 1_750_000_000 * 1000; // fixed "now", minute-aligned

describe('inferStepMs', () => {
  it('returns the step of a uniform series', () => {
    expect(inferStepMs(series({ count: 10, stepMs: MINUTE, endMs: NOW, price: 100 }))).toBe(MINUTE);
  });

  it('is robust to a single gap (median)', () => {
    const c = series({ count: 10, stepMs: MINUTE, endMs: NOW, price: 100 });
    // Knock out one bar to create a 2-minute gap near the front.
    c.splice(2, 1);
    expect(inferStepMs(c)).toBe(MINUTE);
  });

  it('returns null with fewer than two candles', () => {
    expect(inferStepMs([])).toBeNull();
    expect(inferStepMs(series({ count: 1, stepMs: MINUTE, endMs: NOW, price: 100 }))).toBeNull();
  });
});

describe('classifyTransition', () => {
  const btc1m = series({ count: 300, stepMs: MINUTE, endMs: NOW, price: 60_000 });

  it('classifies the first data as initial', () => {
    expect(classifyTransition(null, btc1m, false)).toBe('initial');
    expect(classifyTransition([], btc1m, false)).toBe('initial');
  });

  it('classifies an appended bar as stream', () => {
    const next = [...btc1m, { ...btc1m[btc1m.length - 1], timeMs: NOW + MINUTE }];
    expect(classifyTransition(btc1m, next, false)).toBe('stream');
  });

  it('classifies an updated last bar as stream', () => {
    const next = btc1m.slice(0, -1);
    next.push({ ...btc1m[btc1m.length - 1], close: 60_100 });
    expect(classifyTransition(btc1m, next, false)).toBe('stream');
  });

  it('classifies a rolling buffer (drop oldest, append newest) as stream', () => {
    const next = [...btc1m.slice(1), { ...btc1m[btc1m.length - 1], timeMs: NOW + MINUTE }];
    expect(classifyTransition(btc1m, next, false)).toBe('stream');
  });

  it('classifies a same-step jump of many bars as reset (re-fetch elsewhere)', () => {
    const next = series({ count: 300, stepMs: MINUTE, endMs: NOW + 120 * MINUTE, price: 60_000 });
    expect(classifyTransition(btc1m, next, false)).toBe('reset');
  });

  it('classifies a step change with price/time continuity as timeframe', () => {
    const btc15m = series({ count: 200, stepMs: 15 * MINUTE, endMs: NOW, price: 60_000 });
    expect(classifyTransition(btc1m, btc15m, false)).toBe('timeframe');
    expect(classifyTransition(btc15m, btc1m, false)).toBe('timeframe'); // and back down
  });

  it('classifies a step change with a big price jump as reset (asset switch)', () => {
    const sol15m = series({ count: 200, stepMs: 15 * MINUTE, endMs: NOW, price: 80 });
    expect(classifyTransition(btc1m, sol15m, false)).toBe('reset');
  });

  it('classifies a same-step different-asset swap as reset', () => {
    // Same grid (assets on one exchange share the bar clock): the price
    // discontinuity at the shared last bar decides.
    const sol1m = series({ count: 300, stepMs: MINUTE, endMs: NOW, price: 80 });
    expect(classifyTransition(btc1m, sol1m, false)).toBe('reset');
    // Shifted grid (different exchange clock): alignment decides.
    const solShifted = series({ count: 300, stepMs: MINUTE, endMs: NOW + 30_000, price: 80 });
    expect(classifyTransition(btc1m, solShifted, false)).toBe('reset');
  });

  it('honors seriesKeyChanged over any heuristic', () => {
    const next = [...btc1m, { ...btc1m[btc1m.length - 1], timeMs: NOW + MINUTE }];
    expect(classifyTransition(btc1m, next, true)).toBe('reset');
  });

  it('classifies non-overlapping histories with a step change as reset', () => {
    const old = series({ count: 300, stepMs: MINUTE, endMs: NOW - 500 * MINUTE, price: 60_000 });
    const next = series({ count: 200, stepMs: 15 * MINUTE, endMs: NOW, price: 60_000 });
    expect(classifyTransition(old, next, false)).toBe('reset');
  });

  it('falls back to reset when a series is too short to infer a step', () => {
    const single = series({ count: 1, stepMs: MINUTE, endMs: NOW, price: 60_000 });
    expect(classifyTransition(btc1m, single, false)).toBe('reset');
  });

  it('treats an emptied next array as stream (nothing to reframe)', () => {
    expect(classifyTransition(btc1m, [], false)).toBe('stream');
  });
});

describe('timeframeWindow', () => {
  it('preserves the visible slot count across a 1m -> 15m switch', () => {
    // 100 visible 1m slots ending exactly on the last candle.
    const oldWindow = { startMs: NOW - 100 * MINUTE, endMs: NOW };
    const w = timeframeWindow(oldWindow, MINUTE, NOW, 15 * MINUTE, NOW);
    expect((w.endMs - w.startMs) / (15 * MINUTE)).toBeCloseTo(100);
    expect(w.endMs).toBe(NOW); // right edge anchored on the newest candle
  });

  it('carries the future-gap overshoot over in slots', () => {
    // 100 slots with the last candle 10 slots from the right edge.
    const oldWindow = { startMs: NOW - 90 * MINUTE, endMs: NOW + 10 * MINUTE };
    const w = timeframeWindow(oldWindow, MINUTE, NOW, 15 * MINUTE, NOW);
    expect(w.endMs).toBe(NOW + 10 * 15 * MINUTE);
    expect((w.endMs - w.startMs) / (15 * MINUTE)).toBeCloseTo(100);
  });

  it('snaps a parked-in-history view back to the newest candle', () => {
    // Right edge 50 minutes before the last candle (negative offset).
    const oldWindow = { startMs: NOW - 150 * MINUTE, endMs: NOW - 50 * MINUTE };
    const w = timeframeWindow(oldWindow, MINUTE, NOW, 15 * MINUTE, NOW);
    expect(w.endMs).toBe(NOW); // clamped to zero offset
  });

  it('clamps an oversized future gap to 3/4 of the window', () => {
    // 100 slots, 90 of them future gap (beyond the 75-slot cap).
    const oldWindow = { startMs: NOW - 10 * MINUTE, endMs: NOW + 90 * MINUTE };
    const w = timeframeWindow(oldWindow, MINUTE, NOW, 15 * MINUTE, NOW);
    expect(w.endMs).toBe(NOW + 75 * 15 * MINUTE);
  });
});
