import { describe, expect, it } from 'vitest';
import type { Candle } from '@vroomchart/types';

import { describeChart, findSwingLevels, formatInterval, latestRSI } from './market';

const STEP = 60 * 60_000;
const T0 = Date.UTC(2026, 0, 1);

/** A series that oscillates between ~90 and ~110 with a 12-bar period. */
const ranging: Candle[] = Array.from({ length: 120 }, (_, i) => {
  const mid = 100 + 10 * Math.sin((i / 12) * 2 * Math.PI);
  return { timeMs: T0 + i * STEP, open: mid - 0.2, high: mid + 0.5, low: mid - 0.5, close: mid + 0.2, volume: 1 };
});

describe('findSwingLevels', () => {
  it('finds the repeated range high as resistance and the low as support', () => {
    const levels = findSwingLevels(ranging);
    const res = levels.find((l) => l.kind === 'resistance');
    const sup = levels.find((l) => l.kind === 'support');
    expect(res?.price).toBeCloseTo(110.5, 0);
    expect(sup?.price).toBeCloseTo(89.5, 0);
    expect(res!.touches).toBeGreaterThanOrEqual(5);
    expect(sup!.touches).toBeGreaterThanOrEqual(5);
  });

  it('returns nothing for too few candles', () => {
    expect(findSwingLevels(ranging.slice(0, 4))).toEqual([]);
  });
});

describe('latestRSI', () => {
  it('is 100 for a straight rally and 0 for a straight sell-off', () => {
    const up = Array.from({ length: 30 }, (_, i) => ({ ...ranging[0]!, timeMs: i, close: 100 + i }));
    const down = up.map((c, i) => ({ ...c, close: 100 - i }));
    expect(latestRSI(up)).toBe(100);
    expect(latestRSI(down)).toBe(0);
  });
});

describe('describeChart', () => {
  it('summarizes the interval, window, and enabled indicators', () => {
    const text = describeChart(ranging, {
      rsi: { enabled: true },
      movingAverages: [{ id: 'ma', maType: 'sma', period: 20 }],
      vwap: { enabled: true },
    });
    expect(text).toMatch(/Interval: 1h\. 120 candles loaded/);
    expect(text).toMatch(/In view: 100 bars/);
    expect(text).toMatch(/RSI\(14\) \d/);
    expect(text).toMatch(/SMA\(20\) [\d.]+ \[id ma\]/);
    expect(text).toMatch(/Also shown: VWAP/);
  });

  it('uses the viewport when given', () => {
    const viewport = { startMs: ranging[100]!.timeMs, endMs: ranging[119]!.timeMs };
    expect(describeChart(ranging, {}, viewport)).toMatch(/In view: 20 bars/);
  });

  it('handles an empty chart', () => {
    expect(describeChart([])).toMatch(/no candles/);
  });
});

describe('formatInterval', () => {
  it('formats common steps', () => {
    expect(formatInterval(60_000)).toBe('1m');
    expect(formatInterval(15 * 60_000)).toBe('15m');
    expect(formatInterval(4 * 3_600_000)).toBe('4h');
    expect(formatInterval(86_400_000)).toBe('1d');
  });
});
