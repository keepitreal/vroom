import type { Candle, OrbDirection, OrbTrade } from '@vroomchart/types';
import type { ResolvedOrb } from './config';

export type Levels = { stop: number; target: number | null; risk: number };

/** Stop, target and risk (price distance to the stop) for an entry. */
export function tradeLevels(
  dir: OrbDirection,
  entry: number,
  high: number,
  low: number,
  r: ResolvedOrb,
): Levels {
  const s = dir === 'long' ? 1 : -1;
  let stop: number;
  if (r.stopMode === 'oppositeSide') stop = dir === 'long' ? low : high;
  else if (r.stopMode === 'midpoint') stop = (high + low) / 2;
  else stop = entry - s * r.stopPoints;
  stop -= s * r.stopBufferTicks * r.tickSize;
  const risk = s * (entry - stop);

  let target: number | null = null;
  if (r.targetMode === 'rMultiple') target = entry + s * r.targetR * risk;
  else if (r.targetMode === 'rangeMultiple') {
    target = (dir === 'long' ? high : low) + s * r.rangeMult * (high - low);
  }
  return { stop, target, risk };
}

export type SimContext = {
  candles: readonly Candle[];
  /** One past the session's last bar. */
  dateEnd: number;
  stepMs: number;
  nowMs: number;
  flattenMs: number;
  r: ResolvedOrb;
};

/**
 * Walks closed bars from the entry bar `j` and fills in the exit on `trade`.
 * Returns the exit bar's index, or -1 while the trade is still open.
 *
 * Per bar, in order: a gap through the stop or target fills at the open; a
 * touch of the stop exits there (and wins over a target touched in the same
 * bar, since the intrabar order is unknown); the partial fills; the target
 * fills; then the bar containing `flattenMs` exits at its close.
 */
export function simulateTrade(trade: OrbTrade, j: number, ctx: SimContext): number {
  const { candles, dateEnd, stepMs, nowMs, flattenMs, r } = ctx;
  const s = trade.direction === 'long' ? 1 : -1;
  const risk = s * (trade.entryPrice - trade.stop);
  const partialLevel = trade.entryPrice + s * r.partial.atR * risk;
  // beyond(x, level): price x is at or past level in the trade's favor.
  const beyond = (x: number, level: number) => s * (x - level) >= 0;
  const adverse = (x: number, level: number) => s * (x - level) <= 0;

  const exit = (i: number, price: number, reason: 'target' | 'stop' | 'time', timeMs: number) => {
    trade.exitTimeMs = timeMs;
    trade.exitPrice = price;
    trade.exitReason = reason;
    const rawR = (x: number) => (s * (x - trade.entryPrice)) / risk;
    const p = trade.partial;
    trade.r = p
      ? (p.pct / 100) * rawR(p.price) + (1 - p.pct / 100) * rawR(price)
      : rawR(price);
    return i;
  };

  for (let i = j; i < dateEnd; i++) {
    const c = candles[i];
    if (!(i < candles.length - 1 || nowMs >= c.timeMs + stepMs)) return -1;
    const favorableExtreme = s > 0 ? c.high : c.low;
    const adverseExtreme = s > 0 ? c.low : c.high;

    if (i > j) {
      if (adverse(c.open, trade.currentStop)) return exit(i, c.open, 'stop', c.timeMs);
      if (trade.target != null && beyond(c.open, trade.target)) {
        return exit(i, c.open, 'target', c.timeMs);
      }
    }
    if (adverse(adverseExtreme, trade.currentStop)) {
      return exit(i, trade.currentStop, 'stop', c.timeMs);
    }
    if (r.partial.enabled && !trade.partial && beyond(favorableExtreme, partialLevel)) {
      const price = i > j && beyond(c.open, partialLevel) ? c.open : partialLevel;
      trade.partial = { timeMs: c.timeMs, price, pct: r.partial.fraction * 100 };
      if (r.partial.breakeven) trade.currentStop = trade.entryPrice;
    }
    if (trade.target != null && beyond(favorableExtreme, trade.target)) {
      return exit(i, trade.target, 'target', c.timeMs);
    }
    if (c.timeMs + stepMs > flattenMs) {
      return exit(i, c.close, 'time', c.timeMs + stepMs);
    }
  }

  // The session ended before flattenTime (a half day or missing bars): exit at
  // its last bar once the next session has begun or the flatten time passed.
  const last = dateEnd - 1;
  if (last >= j && (dateEnd < candles.length || nowMs >= flattenMs)) {
    const c = candles[last];
    return exit(last, c.close, 'time', c.timeMs + stepMs);
  }
  return -1;
}
