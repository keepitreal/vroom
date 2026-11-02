import { describe, expect, it } from 'vitest';
import type { Candle } from '@vroomchart/types';
import { classifyDataChange } from './dataChange';

const MINUTE = 60_000;
const NOW = 1_750_000_000 * 1000;

function series(count: number, stepMs: number, price: number): Candle[] {
  return Array.from({ length: count }, (_, i) => ({
    timeMs: NOW - (count - 1 - i) * stepMs,
    open: price,
    close: price * (1 + 0.001 * Math.cos(i)),
    high: price * 1.002,
    low: price * 0.998,
    volume: 100,
  }));
}

describe('classifyDataChange', () => {
  // Live -> 1H: both 1m bars, and the last hour is the tail of the live series,
  // so the heuristics alone call it a stream.
  const live = series(300, MINUTE, 100);
  const hour = live.slice(-60);

  it('morphs a transitionKey change on the same series, step unchanged', () => {
    const prev = { candles: live, seriesKey: 'BTC', transitionKey: 'live' };
    expect(classifyDataChange(prev, { ...prev, candles: hour }, false)).toBe('stream');
    expect(
      classifyDataChange(prev, { candles: hour, seriesKey: 'BTC', transitionKey: '1h' }, false),
    ).toBe('timeframe');
  });

  it('morphs a transitionKey change even with an explicit visibleRange', () => {
    const prev = { candles: live, transitionKey: 0 };
    expect(classifyDataChange(prev, { candles: hour, transitionKey: 0 }, true)).toBe('stream');
    expect(classifyDataChange(prev, { candles: hour, transitionKey: 1 }, true)).toBe('timeframe');
  });

  it('still resets on a seriesKey change', () => {
    const prev = { candles: live, seriesKey: 'BTC', transitionKey: 'live' };
    expect(
      classifyDataChange(prev, { candles: hour, seriesKey: 'ETH', transitionKey: '1h' }, false),
    ).toBe('reset');
  });

  it('falls back to the data heuristics without a key', () => {
    const daily = series(300, 1440 * MINUTE, 100);
    expect(classifyDataChange({ candles: live }, { candles: daily }, false)).toBe('timeframe');
    expect(classifyDataChange({ candles: live }, { candles: daily }, true)).toBe('stream');
  });
});
