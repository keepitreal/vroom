import type { Candle, OrbWarning } from '@vroomchart/types';

/** Chart bars aggregated up to the signal timeframe, on the session clock. */
export type SignalCandle = {
  startMs: number;
  endMs: number;
  open: number;
  high: number;
  low: number;
  close: number;
  /** Index of the last chart bar in it. */
  last: number;
  /** Every bar in it has closed: a later bar exists, or the clock passed its end. */
  closed: boolean;
};

/**
 * Signals need chart bars no longer than the signal timeframe (or the range
 * window), dividing it evenly; otherwise a signal candle can't be built.
 */
export function timeframeWarnings(
  stepMs: number,
  signalTfMin: number,
  windowMin: number,
): OrbWarning[] {
  const signalMs = signalTfMin * 60_000;
  const stepMin = Math.round(stepMs / 60_000);
  if (stepMs > signalMs || stepMs > windowMin * 60_000) {
    return [
      {
        code: 'chartTimeframeTooHigh',
        message: `Chart bars (${stepMin}m) are longer than the ${signalTfMin}m signal timeframe or the ${windowMin}m range; switch to a lower timeframe to see signals.`,
      },
    ];
  }
  if (signalMs % stepMs !== 0) {
    return [
      {
        code: 'signalTimeframeNotMultiple',
        message: `The ${signalTfMin}m signal timeframe isn't a multiple of the ${stepMin}m chart bars; signals are off.`,
      },
    ];
  }
  return [];
}

/**
 * Signal candles from bars `[from, to)` whose bucket starts at or after
 * `fromMs` and before `untilMs`. Buckets align to local midnight, so a 5m
 * candle covers 09:45–09:50 in the session timezone regardless of DST.
 */
export function aggregate(
  candles: readonly Candle[],
  localMinute: readonly number[],
  from: number,
  to: number,
  signalTfMin: number,
  fromMs: number,
  untilMs: number,
  nowMs: number,
): SignalCandle[] {
  const sMs = signalTfMin * 60_000;
  const out: SignalCandle[] = [];
  let cur: SignalCandle | null = null;
  for (let i = from; i < to; i++) {
    const c = candles[i];
    const startMs = c.timeMs - (Math.round(localMinute[i] * 60_000) % sMs);
    if (startMs < fromMs) continue;
    if (startMs >= untilMs) break;
    if (cur && cur.startMs === startMs) {
      if (c.high > cur.high) cur.high = c.high;
      if (c.low < cur.low) cur.low = c.low;
      cur.close = c.close;
      cur.last = i;
      continue;
    }
    cur = {
      startMs,
      endMs: startMs + sMs,
      open: c.open,
      high: c.high,
      low: c.low,
      close: c.close,
      last: i,
      closed: false,
    };
    out.push(cur);
  }
  for (const s of out) s.closed = s.last < candles.length - 1 || nowMs >= s.endMs;
  return out;
}
