import type { Candle } from '@vroomchart/types';
import { Zone } from '../time';

export type OHLC = [open: number, high: number, low: number, close: number];

const zones = new Map<string, Zone>();
const zone = (tz: string) => {
  let z = zones.get(tz);
  if (!z) zones.set(tz, (z = new Zone(tz)));
  return z;
};

/** Epoch ms of `hhmm` on `date` in `tz`. */
export function at(date: string, hhmm: string, tz = 'America/New_York'): number {
  const [h, m] = hhmm.split(':').map(Number);
  return zone(tz).toUtc(date, h * 60 + m);
}

/** A quiet bar inside the default range (20000–20060 / 19980). */
export const FLAT: OHLC = [20010, 20015, 20005, 20010];

/**
 * The default opening range: high 20060, low 19980, midpoint 20020, window
 * open 20000, close 20010 (below the midpoint).
 */
export const RANGE: Record<string, OHLC> = {
  '09:30': [20000, 20050, 19980, 20020],
  '09:35': [20020, 20060, 20000, 20040],
  '09:40': [20040, 20055, 19990, 20010],
};

/**
 * Bars every `stepMin` from `from` up to (not including) `until` on `date`,
 * `FLAT` unless overridden by local time of day.
 */
export function day(
  date: string,
  overrides: Record<string, OHLC> = {},
  opts: { from?: string; until?: string; stepMin?: number; tz?: string; flat?: OHLC } = {},
): Candle[] {
  const { from = '09:30', until = '16:00', stepMin = 5, tz = 'America/New_York', flat = FLAT } = opts;
  const all = { ...RANGE, ...overrides };
  const start = at(date, from, tz);
  const end = at(date, until, tz);
  const out: Candle[] = [];
  for (let t = start; t < end; t += stepMin * 60_000) {
    const l = zone(tz).local(t);
    const hh = String(Math.floor(l.minute / 60)).padStart(2, '0');
    const mm = String(Math.floor(l.minute % 60)).padStart(2, '0');
    const [open, high, low, close] = all[`${hh}:${mm}`] ?? flat;
    out.push({ timeMs: t, open, high, low, close, volume: 1 });
  }
  return out;
}
