import type { Candle, OrbSkipReason } from '@vroomchart/types';
import type { ResolvedOrb } from './config';
import type { Zone } from './time';

/** A skip that's known before the range forms (calendar filters). */
export function calendarSkip(
  date: string,
  weekday: number,
  r: ResolvedOrb,
): OrbSkipReason | null {
  if (r.skipDates.has(date)) return 'skipDate';
  if (r.skipWeekdays.has(weekday)) return 'skipWeekday';
  return null;
}

export function exceedsMaxWidth(widthPct: number, r: ResolvedOrb): boolean {
  return r.maxRangeWidthPct != null && widthPct > r.maxRangeWidthPct;
}

/**
 * Daily closes keyed by session date, oldest first. Uses `trend.dailyCloses`
 * when given, else the last close of each date in the chart data.
 */
export function dailyCloses(
  candles: readonly Candle[],
  dates: readonly string[],
  zone: Zone,
  r: ResolvedOrb,
): { date: string; close: number }[] {
  const byDate = new Map<string, number>();
  if (r.trend.dailyCloses) {
    const sorted = [...r.trend.dailyCloses].sort((a, b) => a.timeMs - b.timeMs);
    for (const d of sorted) byDate.set(zone.local(d.timeMs).date, d.close);
  } else {
    for (let i = 0; i < candles.length; i++) byDate.set(dates[i], candles[i].close);
  }
  return [...byDate.entries()]
    .sort((a, b) => (a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0))
    .map(([date, close]) => ({ date, close }));
}

/**
 * SMA of the `period` daily closes strictly before `date`, or null when there
 * are fewer. Only prior days count, so the filter never sees the session's own close.
 */
export function priorSma(
  closes: readonly { date: string; close: number }[],
  date: string,
  period: number,
): number | null {
  let end = 0;
  while (end < closes.length && closes[end].date < date) end++;
  if (end < period) return null;
  let sum = 0;
  for (let i = end - period; i < end; i++) sum += closes[i].close;
  return sum / period;
}
