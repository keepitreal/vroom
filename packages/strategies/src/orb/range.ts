import type { Candle } from '@vroomchart/types';

export type RangeState =
  | { kind: 'waiting' }
  | { kind: 'incomplete' }
  | {
      kind: 'forming' | 'final';
      high: number;
      low: number;
      open: number;
      close: number;
      /** When the range is complete: the window end, or the last window bar's close if later. */
      finalMs: number;
    };

/**
 * The opening range from bars that open in `[startMs, endMs)`. A range is
 * incomplete when the data starts after the window opened (it would be built
 * from a partial window) or when the window passed with no bars in it.
 */
export function buildRange(
  candles: readonly Candle[],
  from: number,
  to: number,
  startMs: number,
  endMs: number,
  stepMs: number,
  nowMs: number,
): RangeState {
  const n = candles.length;
  if (n === 0) return { kind: 'waiting' };
  if (candles[0].timeMs > startMs) return { kind: 'incomplete' };

  let high = -Infinity;
  let low = Infinity;
  let first = -1;
  let last = -1;
  for (let i = from; i < to; i++) {
    const t = candles[i].timeMs;
    if (t < startMs) continue;
    if (t >= endMs) break;
    if (first < 0) first = i;
    last = i;
    if (candles[i].high > high) high = candles[i].high;
    if (candles[i].low < low) low = candles[i].low;
  }

  if (first < 0) {
    const passed = to < n || candles[to - 1].timeMs >= endMs || nowMs >= endMs;
    return passed ? { kind: 'incomplete' } : { kind: 'waiting' };
  }

  const finalMs = Math.max(endMs, candles[last].timeMs + stepMs);
  const final = last < n - 1 || nowMs >= finalMs;
  return {
    kind: final ? 'final' : 'forming',
    high,
    low,
    open: candles[first].open,
    close: candles[last].close,
    finalMs,
  };
}
