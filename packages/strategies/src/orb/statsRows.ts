import type { OrbResult, OrbSkipReason } from '@vroomchart/types';

const SKIP_LABEL: Record<OrbSkipReason, string> = {
  incompleteRange: 'incomplete range',
  maxRangeWidth: 'range too wide',
  skipDate: 'date filter',
  skipWeekday: 'weekday filter',
  trendInsufficientHistory: 'trend history',
};

const r = (v: number) => `${v >= 0 ? '+' : ''}${v.toFixed(2)}R`;

/**
 * The stats table as `[label, value]` rows, for a panel to lay out; shared so
 * every platform shows the same numbers the same way.
 */
export function orbStatsRows(result: OrbResult): [string, string][] {
  const s = result.stats;
  const rows: [string, string][] = [
    ['Trades', String(s.trades)],
    ['Win rate', s.winRate == null ? '–' : `${Math.round(s.winRate * 100)}%`],
    ['Avg R', s.avgR == null ? '–' : r(s.avgR)],
    ['Net R', r(s.netR)],
    ['Profit factor', s.profitFactor == null ? '–' : s.profitFactor.toFixed(2)],
    ['Max DD', `${s.maxDrawdownR.toFixed(2)}R`],
    ['Longs', `${s.longs.trades} (${s.longs.wins} W, ${r(s.longs.netR)})`],
    ['Shorts', `${s.shorts.trades} (${s.shorts.wins} W, ${r(s.shorts.netR)})`],
    ['Sessions', String(s.sessions)],
  ];
  for (const [reason, n] of Object.entries(s.skipped) as [OrbSkipReason, number][]) {
    rows.push([`Skipped: ${SKIP_LABEL[reason]}`, String(n)]);
  }
  return rows;
}
