import type {
  Candle,
  OrbConfig,
  OrbDirection,
  OrbResult,
  OrbSession,
  OrbSignalStatus,
  OrbTrade,
  OrbWarning,
} from '@vroomchart/types';
import { resolveOrb, type ResolvedOrb } from './config';
import { EventLog } from './events';
import { calendarSkip, dailyCloses, exceedsMaxWidth, priorSma } from './filters';
import { buildRange } from './range';
import { aggregate, timeframeWarnings } from './signals';
import { orbStats } from './stats';
import { simulateTrade, suggestedSize, tradeLevels } from './trade';
import { Zone } from './time';

export type RunOrbOptions = {
  /**
   * Wall-clock time (epoch ms) deciding whether the newest bar has closed.
   * Defaults to +Infinity: every bar is closed, as in a backtest. Pass
   * `Date.now()` on a live chart so the forming bar is never traded.
   */
  nowMs?: number;
  /** Bar length in ms. Inferred from the bar spacing when omitted. */
  stepMs?: number;
};

/** The most common gap between consecutive bars (robust to holes in the data). */
export function inferStepMs(candles: readonly Candle[]): number {
  const counts = new Map<number, number>();
  const end = Math.min(candles.length, 500);
  for (let i = 1; i < end; i++) {
    const d = candles[i].timeMs - candles[i - 1].timeMs;
    if (d > 0) counts.set(d, (counts.get(d) ?? 0) + 1);
  }
  let best = 60_000;
  let bestCount = 0;
  for (const [d, c] of counts) {
    if (c > bestCount || (c === bestCount && d < best)) {
      best = d;
      bestCount = c;
    }
  }
  return best;
}

/**
 * Runs the Opening Range Breakout assistant over `candles` (oldest first).
 * Pure and deterministic: the same bars, config and `nowMs` always give the
 * same result, and nothing about a session depends on bars after the one
 * being evaluated.
 */
export function runOrb(
  candles: readonly Candle[],
  config?: OrbConfig,
  opts: RunOrbOptions = {},
): OrbResult {
  const r = resolveOrb(config);
  const nowMs = opts.nowMs ?? Infinity;
  const empty = (warnings: OrbWarning[]): OrbResult => ({
    sessions: [],
    trades: [],
    events: [],
    stats: orbStats([], []),
    warnings,
    signalsEnabled: false,
  });

  let zone: Zone;
  try {
    zone = new Zone(r.timezone);
  } catch {
    return empty([
      { code: 'invalidTimezone', message: `Unknown timezone '${r.timezone}'; the assistant is off.` },
    ]);
  }
  if (candles.length === 0) return empty([]);

  const stepMs = opts.stepMs ?? inferStepMs(candles);
  const warnings = timeframeWarnings(stepMs, r.signalTfMin, r.rangeEndMin - r.rangeStartMin);
  const signalsEnabled = warnings.length === 0;

  const n = candles.length;
  const dates: string[] = new Array(n);
  const minutes: number[] = new Array(n);
  const weekdays: number[] = new Array(n);
  for (let i = 0; i < n; i++) {
    const l = zone.local(candles[i].timeMs);
    dates[i] = l.date;
    minutes[i] = l.minute;
    weekdays[i] = l.weekday;
  }
  const closes = r.trend.enabled ? dailyCloses(candles, dates, zone, r) : [];

  const log = new EventLog();
  const sessions: OrbSession[] = [];
  const trades: OrbTrade[] = [];

  for (let a = 0; a < n; ) {
    let b = a + 1;
    while (b < n && dates[b] === dates[a]) b++;
    const date = dates[a];
    const session: OrbSession = {
      date,
      rangeStartMs: zone.toUtc(date, r.rangeStartMin),
      rangeEndMs: zone.toUtc(date, r.rangeEndMin),
      entryCutoffMs: zone.toUtc(date, r.cutoffMin),
      flattenMs: zone.toUtc(date, r.flattenMin),
      status: 'forming',
      signals: [],
      trades: [],
    };
    sessions.push(session);
    const skip = (reason: NonNullable<OrbSession['skipReason']>, timeMs: number) => {
      session.status = 'skipped';
      session.skipReason = reason;
      log.add({ type: 'daySkipped', timeMs, date, reason }, reason);
    };

    const cal = calendarSkip(date, weekdays[a], r);
    if (cal) {
      skip(cal, session.rangeStartMs);
      a = b;
      continue;
    }

    const range = buildRange(candles, a, b, session.rangeStartMs, session.rangeEndMs, stepMs, nowMs);
    if (range.kind === 'incomplete') skip('incompleteRange', session.rangeEndMs);
    if (range.kind === 'forming' || range.kind === 'final') {
      const mid = (range.high + range.low) / 2;
      session.high = range.high;
      session.low = range.low;
      session.open = range.open;
      session.close = range.close;
      session.widthPct = mid > 0 ? ((range.high - range.low) / mid) * 100 : 0;
    }
    if (range.kind === 'final') {
      session.finalMs = range.finalMs;
      session.status = 'final';
      log.add({ type: 'rangeFinalized', timeMs: range.finalMs, date, price: (range.high + range.low) / 2 }, 'range');
      let sma: number | null = null;
      if (exceedsMaxWidth(session.widthPct!, r)) skip('maxRangeWidth', range.finalMs);
      else if (r.trend.enabled) {
        sma = priorSma(closes, date, r.trend.period);
        if (sma == null) skip('trendInsufficientHistory', range.finalMs);
      }
      if (session.status === 'final' && signalsEnabled) {
        runSession(session, { candles, minutes, a, b, stepMs, nowMs, sma, r, log });
        trades.push(...session.trades);
      }
    }
    a = b;
  }

  return {
    sessions,
    trades,
    events: log.sorted(),
    stats: orbStats(sessions, trades),
    warnings,
    signalsEnabled,
  };
}

type SessionContext = {
  candles: readonly Candle[];
  minutes: readonly number[];
  a: number;
  b: number;
  stepMs: number;
  nowMs: number;
  sma: number | null;
  r: ResolvedOrb;
  log: EventLog;
};

/** Confirmations, entries and exits for one final, unskipped session. */
function runSession(session: OrbSession, ctx: SessionContext): void {
  const { candles, minutes, a, b, stepMs, nowMs, sma, r, log } = ctx;
  const { date } = session;
  const high = session.high!;
  const low = session.low!;
  const mid = (high + low) / 2;
  const buf =
    r.breakUnit === 'ticks'
      ? r.breakDistance * r.tickSize
      : r.breakUnit === 'points'
        ? r.breakDistance
        : (r.breakDistance / 100) * (high - low);

  const allowed = (dir: OrbDirection): boolean => {
    switch (r.directionMode) {
      case 'longOnly':
        return dir === 'long';
      case 'shortOnly':
        return dir === 'short';
      case 'withRangeCandle':
        return dir === 'long' ? session.close! > mid : session.close! < mid;
      default:
        return true;
    }
  };

  const signalCandles = aggregate(
    candles, minutes, a, b, r.signalTfMin, session.finalMs!, session.flattenMs, nowMs,
  );
  const sideSignals = { long: 0, short: 0 };
  const maxSide = { long: r.maxLongSignals, short: r.maxShortSignals };
  let longCount = 0;
  let shortCount = 0;
  let tradeCount = 0;
  let lostDir: OrbDirection | null = null;
  let reversalUsed = false;
  let resumeAfterMs = -Infinity;

  for (const sc of signalCandles) {
    if (sc.endMs <= resumeAfterMs) continue;
    const longBeyond = sc.close > high + buf;
    const shortBeyond = sc.close < low - buf;

    if (!sc.closed) {
      const dir: OrbDirection | null = longBeyond ? 'long' : shortBeyond ? 'short' : null;
      if (dir && allowed(dir) && sc.endMs < session.entryCutoffMs && tradeCount < r.maxTradesPerDay) {
        session.pending = {
          direction: dir,
          level: dir === 'long' ? high : low,
          price: sc.close,
          timeMs: sc.startMs,
        };
      }
      break;
    }

    longCount = longBeyond ? longCount + 1 : 0;
    shortCount = shortBeyond ? shortCount + 1 : 0;
    const dir: OrbDirection | null =
      longCount >= r.confirmCloses ? 'long' : shortCount >= r.confirmCloses ? 'short' : null;
    if (!dir) continue;
    if (dir === 'long') longCount = 0;
    else shortCount = 0;
    if (!allowed(dir)) continue;

    const record = (status: OrbSignalStatus, extra: { tradeId?: string; size?: number } = {}) => {
      session.signals.push({
        direction: dir,
        timeMs: sc.endMs,
        level: dir === 'long' ? high : low,
        close: sc.close,
        status,
        ...extra,
      });
    };
    const confirm = () => {
      sideSignals[dir]++;
      log.add(
        { type: 'breakoutConfirmed', timeMs: sc.endMs, date, direction: dir, price: sc.close },
        `${dir}:${sc.endMs}`,
      );
    };

    const j = sc.last + 1;
    if (sc.endMs >= session.entryCutoffMs || (j < b && candles[j].timeMs >= session.entryCutoffMs)) {
      record('afterCutoff');
      break;
    }
    if (sma != null && (dir === 'long' ? sc.close <= sma : sc.close >= sma)) {
      record('blockedByTrend');
      continue;
    }
    const reversal = lostDir != null && r.allowReversal && !reversalUsed && dir !== lostDir;
    if (lostDir != null && !reversal) {
      record('blockedAfterLoss');
      continue;
    }
    if (sideSignals[dir] >= maxSide[dir]) {
      record('maxSignals');
      continue;
    }
    if (tradeCount >= r.maxTradesPerDay) {
      record('maxTrades');
      continue;
    }
    if (j >= candles.length) {
      record('awaitingEntry');
      confirm();
      break;
    }
    if (j >= b) {
      record('afterCutoff');
      break;
    }

    const entryBar = candles[j];
    const entry = entryBar.open;
    const lv = tradeLevels(dir, entry, high, low, r);
    if (!(lv.risk > 0)) {
      record('invalidStop');
      confirm();
      continue;
    }
    const size = suggestedSize(lv.risk, r);
    if (size < 1) {
      record('sizeTooSmall', { size });
      confirm();
      continue;
    }

    const trade: OrbTrade = {
      id: `${date}:${dir}:${entryBar.timeMs}`,
      date,
      direction: dir,
      signalTimeMs: sc.endMs,
      entryTimeMs: entryBar.timeMs,
      entryPrice: entry,
      stop: lv.stop,
      currentStop: lv.stop,
      target: lv.target,
      size,
      exitTimeMs: null,
      exitPrice: null,
      exitReason: null,
      r: null,
    };
    record('entered', { tradeId: trade.id, size });
    confirm();
    tradeCount++;
    if (reversal) reversalUsed = true;
    session.trades.push(trade);
    log.add(
      { type: 'entry', timeMs: entryBar.timeMs, date, direction: dir, price: entry, tradeId: trade.id },
      trade.id,
    );

    const exitIdx = simulateTrade(trade, j, {
      candles, dateEnd: b, stepMs, nowMs, flattenMs: session.flattenMs, r,
    });
    if (exitIdx < 0) break;
    const type =
      trade.exitReason === 'target' ? 'targetHit' : trade.exitReason === 'stop' ? 'stopHit' : 'timeExit';
    log.add(
      {
        type,
        timeMs: trade.exitTimeMs!,
        date,
        direction: dir,
        price: trade.exitPrice!,
        tradeId: trade.id,
        r: trade.r!,
      },
      trade.id,
    );
    if (trade.exitReason === 'stop' && trade.r! < 0 && r.stopAfterLoss) lostDir = dir;
    resumeAfterMs = candles[exitIdx].timeMs + stepMs;
    longCount = 0;
    shortCount = 0;
  }
}
