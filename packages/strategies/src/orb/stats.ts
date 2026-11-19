import type { OrbSession, OrbStats, OrbTrade } from '@vroomchart/types';

/** Performance over closed trades, in R. Open trades are left out. */
export function orbStats(sessions: readonly OrbSession[], trades: readonly OrbTrade[]): OrbStats {
  const closed = trades.filter((t) => t.r != null);
  const side = (dir: 'long' | 'short') => {
    const ts = closed.filter((t) => t.direction === dir);
    return {
      trades: ts.length,
      wins: ts.filter((t) => t.r! > 0).length,
      netR: ts.reduce((a, t) => a + t.r!, 0),
    };
  };

  let netR = 0;
  let peak = 0;
  let maxDrawdownR = 0;
  let grossWin = 0;
  let grossLoss = 0;
  let wins = 0;
  let losses = 0;
  for (const t of closed) {
    const r = t.r!;
    netR += r;
    peak = Math.max(peak, netR);
    maxDrawdownR = Math.max(maxDrawdownR, peak - netR);
    if (r > 0) {
      wins++;
      grossWin += r;
    } else if (r < 0) {
      losses++;
      grossLoss -= r;
    }
  }

  const skipped: OrbStats['skipped'] = {};
  for (const s of sessions) {
    if (s.status === 'skipped' && s.skipReason) {
      skipped[s.skipReason] = (skipped[s.skipReason] ?? 0) + 1;
    }
  }

  return {
    trades: closed.length,
    wins,
    losses,
    winRate: closed.length ? wins / closed.length : null,
    avgR: closed.length ? netR / closed.length : null,
    netR,
    profitFactor: grossLoss > 0 ? grossWin / grossLoss : null,
    maxDrawdownR,
    longs: side('long'),
    shorts: side('short'),
    sessions: sessions.filter((s) => s.status !== 'forming').length,
    skipped,
  };
}
