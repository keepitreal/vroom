import { describe, expect, it } from 'vitest';
import type { Candle, OrbConfig } from '@vroomchart/types';
import { resolveOrb } from '../config';
import { runOrb } from '../run';
import { tradeLevels } from '../trade';
import { at, day, type OHLC } from './helpers';

const D = '2026-06-01'; // a Monday
const STEP = 5 * 60_000;

/** A close above the range high (20060) on the 09:45 candle, then a calm entry bar. */
const LONG_BREAK: Record<string, OHLC> = {
  '09:45': [20012, 20075, 20010, 20070],
  '09:50': [20072, 20080, 20065, 20075],
};

const session = (candles: Candle[], cfg?: OrbConfig, nowMs?: number) =>
  runOrb(candles, cfg, { nowMs }).sessions[0];

describe('range', () => {
  it('uses only bars inside [rangeStart, rangeEnd)', () => {
    const bars = day(D, { '09:25': [20000, 30000, 10000, 20000], '09:45': [20010, 30000, 10000, 20010] }, { from: '09:00' });
    const s = session(bars);
    expect(s.high).toBe(20060);
    expect(s.low).toBe(19980);
    expect(s.open).toBe(20000);
    expect(s.close).toBe(20010);
    expect(s.finalMs).toBe(at(D, '09:45'));
  });

  it('follows DST: 09:30 New York is 14:30 UTC in winter and 13:30 UTC in summer', () => {
    expect(session(day('2026-03-06')).rangeStartMs).toBe(Date.UTC(2026, 2, 6, 14, 30));
    expect(session(day('2026-03-09')).rangeStartMs).toBe(Date.UTC(2026, 2, 9, 13, 30));
  });

  it('builds the range on the DST-change day itself', () => {
    for (const [date, utcHour] of [['2026-03-08', 13], ['2026-11-01', 14]] as const) {
      const bars = day(date, { '09:25': [20000, 30000, 10000, 20000] }, { from: '00:00' });
      const s = session(bars);
      expect(s.rangeStartMs).toBe(Date.UTC(2026, Number(date.slice(5, 7)) - 1, Number(date.slice(8)), utcHour, 30));
      expect([s.high, s.low]).toEqual([20060, 19980]);
    }
  });

  it('works in other timezones', () => {
    const tz = 'Europe/London';
    const bars = day(D, { '08:00': [100, 101, 99, 100.5] }, { from: '08:00', until: '09:00', tz, flat: [100, 100.2, 99.8, 100] });
    const s = session(bars, { session: { timezone: tz, rangeStart: '08:00', rangePreset: '5m' } });
    expect(s.rangeStartMs).toBe(Date.UTC(2026, 5, 1, 7, 0));
    expect([s.high, s.low]).toEqual([101, 99]);
  });

  it('skips a range when the chart starts mid-window', () => {
    const bars = [...day('2026-05-29').filter((c) => c.timeMs >= at('2026-05-29', '09:35')), ...day(D)];
    const res = runOrb(bars);
    expect(res.sessions[0]).toMatchObject({ status: 'skipped', skipReason: 'incompleteRange' });
    expect(res.sessions[1].status).toBe('final');
    expect(res.events.find((e) => e.type === 'daySkipped')?.reason).toBe('incompleteRange');
  });

  it('has no session on a holiday with no bars', () => {
    const res = runOrb([...day('2026-05-22'), ...day('2026-05-26')]);
    expect(res.sessions.map((s) => s.date)).toEqual(['2026-05-22', '2026-05-26']);
  });

  it('is forming until the window closes', () => {
    const bars = day(D, {}, { until: '09:40' });
    const s = session(bars, undefined, at(D, '09:41'));
    expect(s.status).toBe('forming');
    expect(s.high).toBe(20060);
  });
});

describe('confirmation', () => {
  it('ignores a wick through the level when the close is inside', () => {
    const s = session(day(D, { '09:50': [20010, 20120, 20000, 20055], '10:00': [20000, 20010, 19900, 19985] }));
    expect(s.signals).toEqual([]);
    expect(s.trades).toEqual([]);
  });

  it('confirmCloses = 2 needs two consecutive closes beyond the level', () => {
    const s = session(
      day(D, {
        '09:45': [20010, 20075, 20005, 20070], // beyond
        '09:50': [20070, 20072, 20030, 20040], // back inside: reset
        '09:55': [20040, 20075, 20035, 20070], // beyond (1)
        '10:00': [20070, 20090, 20065, 20085], // beyond (2): confirmed at 10:05
        '10:05': [20086, 20090, 20080, 20088],
      }),
      { confirmation: { confirmCloses: 2 } },
    );
    expect(s.signals).toHaveLength(1);
    expect(s.signals[0]).toMatchObject({ direction: 'long', timeMs: at(D, '10:05'), status: 'entered' });
    expect(s.trades[0].entryTimeMs).toBe(at(D, '10:05'));
  });

  it('enters at the next bar open, not the confirming close', () => {
    const t = session(day(D, LONG_BREAK)).trades[0];
    expect(t.signalTimeMs).toBe(at(D, '09:50'));
    expect(t.entryTimeMs).toBe(at(D, '09:50'));
    expect(t.entryPrice).toBe(20072);
  });

  it('aggregates 1m chart bars into 5m signal candles', () => {
    const bars = day(
      D,
      { '09:47': [20010, 20075, 20005, 20070], '09:54': [20010, 20075, 20005, 20070] },
      { stepMin: 1 },
    );
    const s = session(bars);
    // 09:47's close is mid-candle; the 09:50–09:55 candle closes at 09:54's 20070.
    expect(s.signals).toHaveLength(1);
    expect(s.signals[0].timeMs).toBe(at(D, '09:55'));
    expect(s.trades[0].entryTimeMs).toBe(at(D, '09:55'));
  });

  it('applies minBreakDistance in each unit', () => {
    const bars = day(D, { '09:45': [20012, 20070, 20010, 20063] });
    const signals = (minBreakDistance: number, minBreakUnit: 'ticks' | 'points' | 'rangePct') =>
      session(bars, { confirmation: { minBreakDistance, minBreakUnit } }).signals.length;
    expect(signals(20, 'ticks')).toBe(0); // needs > 20065
    expect(signals(2, 'points')).toBe(1); // needs > 20062
    expect(signals(10, 'rangePct')).toBe(0); // needs > 20068
  });

  it('shows a forming candle beyond the level as pending, without signals or events', () => {
    const bars = day(D, { '09:45': [20012, 20075, 20010, 20070] }, { until: '09:50' });
    const res = runOrb(bars, undefined, { nowMs: at(D, '09:47') });
    expect(res.sessions[0].pending).toMatchObject({ direction: 'long', level: 20060, price: 20070 });
    expect(res.sessions[0].signals).toEqual([]);
    expect(res.events.map((e) => e.type)).toEqual(['rangeFinalized']);
  });

  it('warns and disables signals when the chart timeframe is too high or not a divisor', () => {
    const tooHigh = runOrb(day(D, LONG_BREAK, { stepMin: 15 }));
    expect(tooHigh.warnings[0].code).toBe('chartTimeframeTooHigh');
    expect(tooHigh.signalsEnabled).toBe(false);
    expect(tooHigh.trades).toEqual([]);
    expect(tooHigh.sessions[0].high).toBeDefined();

    const odd = runOrb(day(D, {}, { stepMin: 3 }));
    expect(odd.warnings[0].code).toBe('signalTimeframeNotMultiple');
    expect(odd.signalsEnabled).toBe(false);
  });

  it('warns on an unknown timezone', () => {
    const res = runOrb(day(D), { session: { timezone: 'Mars/Olympus' } });
    expect(res.warnings[0].code).toBe('invalidTimezone');
    expect(res.sessions).toEqual([]);
  });
});

describe('exits', () => {
  it('records a bar touching both stop and target as a stop', () => {
    const t = session(day(D, { ...LONG_BREAK, '09:55': [20072, 20300, 19970, 20100] })).trades[0];
    expect(t.target).toBe(20256);
    expect(t.exitReason).toBe('stop');
    expect(t.exitPrice).toBe(19980);
    expect(t.r).toBe(-1);
  });

  it('fills a gap through the stop at the open', () => {
    const t = session(day(D, { ...LONG_BREAK, '09:55': [19950, 19960, 19940, 19955] })).trades[0];
    expect(t.exitReason).toBe('stop');
    expect(t.exitPrice).toBe(19950);
    expect(t.r).toBeCloseTo((19950 - 20072) / 92);
  });

  it('exits at the flatten bar close with the right R', () => {
    const bars = day(D, { ...LONG_BREAK, '15:55': [20090, 20110, 20085, 20100] });
    const t = session(bars, { risk: { targetMode: 'holdToFlatten' } }).trades[0];
    expect(t).toMatchObject({ exitReason: 'time', exitPrice: 20100, exitTimeMs: at(D, '16:00') });
    expect(t.r).toBeCloseTo(28 / 92);

    const early = session(bars, { session: { flattenTime: '15:52' }, risk: { targetMode: 'holdToFlatten' } }).trades[0];
    expect(early).toMatchObject({ exitPrice: 20010, exitTimeMs: at(D, '15:55') });
  });

  it('exits at the last bar of a half day once the next session starts', () => {
    const bars = [...day(D, LONG_BREAK, { until: '13:00' }), ...day('2026-06-02')];
    const t = runOrb(bars, { risk: { targetMode: 'holdToFlatten' } }).trades[0];
    expect(t).toMatchObject({ exitReason: 'time', exitTimeMs: at(D, '13:00') });
  });

  it('takes a partial and moves the stop to breakeven', () => {
    const t = session(
      day(D, {
        ...LONG_BREAK,
        '09:55': [20080, 20170, 20075, 20160], // partial at 1R = 20164
        '10:00': [20150, 20155, 20060, 20070], // back to the entry
      }),
      { risk: { partial: { enabled: true } } },
    ).trades[0];
    expect(t.partial).toEqual({ timeMs: at(D, '09:55'), price: 20164, pct: 50 });
    expect(t.currentStop).toBe(20072);
    expect(t).toMatchObject({ exitReason: 'stop', exitPrice: 20072 });
    expect(t.r).toBeCloseTo(0.5);
  });
});

describe('levels', () => {
  const base = (risk: OrbConfig['risk']) => resolveOrb({ risk });
  it('computes every stop mode, with a tick buffer', () => {
    expect(tradeLevels('long', 20072, 20060, 19980, base({ stopBufferTicks: 2 }))).toEqual({ stop: 19979.5, risk: 92.5, target: 20257 });
    expect(tradeLevels('long', 20072, 20060, 19980, base({ stopMode: 'midpoint' }))).toEqual({ stop: 20020, risk: 52, target: 20176 });
    expect(tradeLevels('long', 20072, 20060, 19980, base({ stopMode: 'fixedPoints', stopPoints: 10 }))).toEqual({ stop: 20062, risk: 10, target: 20092 });
    expect(tradeLevels('short', 19970, 20060, 19980, base({}))).toEqual({ stop: 20060, risk: 90, target: 19790 });
  });

  it('computes every target mode', () => {
    expect(tradeLevels('long', 20072, 20060, 19980, base({ targetMode: 'holdToFlatten' })).target).toBeNull();
    expect(tradeLevels('long', 20072, 20060, 19980, base({ targetMode: 'rMultiple', targetR: 3 })).target).toBe(20348);
    expect(tradeLevels('long', 20072, 20060, 19980, base({ targetMode: 'rangeMultiple', rangeMult: 1.5 })).target).toBe(20180);
    expect(tradeLevels('short', 19970, 20060, 19980, base({ targetMode: 'rangeMultiple', rangeMult: 1.5 })).target).toBe(19860);
  });

  it('makes no sizing assumptions: a trade carries prices and R only', () => {
    const t = session(day(D, LONG_BREAK)).trades[0];
    expect(t).not.toHaveProperty('size');
    expect(t.entryPrice - t.stop).toBeGreaterThan(0);
  });
});

describe('filters and limits', () => {
  it('maxRangeWidthPct skips a wide range', () => {
    const res = runOrb(day(D, { ...LONG_BREAK, '09:40': [20040, 20055, 19500, 20010] }));
    expect(res.sessions[0]).toMatchObject({ status: 'skipped', skipReason: 'maxRangeWidth' });
    expect(res.trades).toEqual([]);
    expect(res.stats.skipped).toEqual({ maxRangeWidth: 1 });
    const off = runOrb(day(D, { ...LONG_BREAK, '09:40': [20040, 20055, 19500, 20010] }), {
      filters: { maxRangeWidthPct: false },
    });
    expect(off.trades).toHaveLength(1);
  });

  it('skipDates and skipWeekdays skip the day', () => {
    expect(session(day(D, LONG_BREAK), { filters: { skipDates: [D] } })).toMatchObject({ status: 'skipped', skipReason: 'skipDate', trades: [] });
    expect(session(day(D, LONG_BREAK), { filters: { skipWeekdays: [1] } })).toMatchObject({ status: 'skipped', skipReason: 'skipWeekday' });
  });

  it('entryCutoff blocks late breakouts', () => {
    const late = session(day(D, { '11:55': [20010, 20080, 20005, 20070] }));
    expect(late.signals[0].status).toBe('afterCutoff');
    expect(late.trades).toEqual([]);
    expect(session(day(D, { '11:50': [20010, 20080, 20005, 20070] })).trades).toHaveLength(1);
  });

  it('maxTradesPerDay and per-side caps block further trades', () => {
    const bars = day(D, {
      '09:45': [20012, 20075, 20010, 20070],
      '09:50': [20072, 20260, 20070, 20250], // target 20256 hit: +2R
      '10:30': [20010, 20080, 20005, 20070], // second long break
    });
    const a = session(bars);
    expect(a.trades[0]).toMatchObject({ exitReason: 'target', r: 2 });
    expect(a.signals.map((s) => s.status)).toEqual(['entered', 'maxSignals']);
    const b = session(bars, { confirmation: { maxBullishSignals: 2 } });
    expect(b.signals.map((s) => s.status)).toEqual(['entered', 'maxTrades']);
    const c = session(bars, { confirmation: { maxBullishSignals: 2, maxTradesPerDay: 2 } });
    expect(c.trades).toHaveLength(2);
  });

  it('withRangeCandle only trades with the range candle', () => {
    const shortBreak: Record<string, OHLC> = { '10:00': [19990, 19995, 19960, 19970] };
    // The default window closes at 20010, below its 20020 midpoint: shorts only.
    const bearish = session(day(D, { ...LONG_BREAK, ...shortBreak }), {
      confirmation: { directionMode: 'withRangeCandle', maxTradesPerDay: 2 },
      risk: { targetMode: 'holdToFlatten' },
    });
    expect(bearish.signals.map((s) => s.direction)).toEqual(['short']);

    const bullishRange = { '09:40': [20040, 20055, 19990, 20050] as OHLC };
    const bullish = session(day(D, { ...bullishRange, '09:45': [20050, 20055, 20030, 20040], ...shortBreak }), {
      confirmation: { directionMode: 'withRangeCandle' },
    });
    expect(bullish.signals).toEqual([]);
  });

  it('stopAfterLoss blocks re-entry; allowReversal permits one opposite-side trade', () => {
    const bars = day(D, {
      '09:45': [20012, 20075, 20010, 20070],
      '09:50': [20072, 20080, 19975, 19990], // stopped out on the entry bar
      '10:00': [19990, 19995, 19960, 19970], // short break
    });
    const blocked = session(bars, { confirmation: { maxTradesPerDay: 2 } });
    expect(blocked.signals.map((s) => s.status)).toEqual(['entered', 'blockedAfterLoss']);
    const reversed = session(bars, { confirmation: { maxTradesPerDay: 2 }, risk: { allowReversal: true } });
    expect(reversed.trades.map((t) => t.direction)).toEqual(['long', 'short']);
    expect(reversed.trades[0].r).toBe(-1);
  });

  it('trend filter uses only prior days’ closes', () => {
    const daily = (closes: number[]) => [
      ...closes.map((close, i) => ({ timeMs: at(`2026-05-2${6 + i}`, '16:00'), close })),
      { timeMs: at(D, '16:00'), close: 1e9 }, // the session's own close must be ignored
    ];
    const cfg = (closes: number[], period = 3): OrbConfig => ({
      filters: { trendFilter: { enabled: true, period, dailyCloses: daily(closes) } },
    });
    expect(session(day(D, LONG_BREAK), cfg([20000, 20000, 20000])).trades).toHaveLength(1);
    expect(session(day(D, LONG_BREAK), cfg([20100, 20100, 20100])).signals[0].status).toBe('blockedByTrend');
    expect(session(day(D, LONG_BREAK), cfg([20000, 20000, 20000], 5))).toMatchObject({ status: 'skipped', skipReason: 'trendInsufficientHistory' });
  });
});

describe('stats and events', () => {
  it('summarizes trades in R', () => {
    const win = day('2026-05-28', {
      '09:45': [20012, 20075, 20010, 20070],
      '09:50': [20072, 20260, 20070, 20250],
    });
    const loss = day('2026-05-29', { ...LONG_BREAK, '09:55': [20072, 20080, 19970, 19990] });
    const wide = day(D, { '09:40': [20040, 20055, 19500, 20010] });
    const { stats } = runOrb([...win, ...loss, ...wide]);
    expect(stats).toMatchObject({
      trades: 2,
      wins: 1,
      losses: 1,
      winRate: 0.5,
      avgR: 0.5,
      netR: 1,
      profitFactor: 2,
      maxDrawdownR: 1,
      longs: { trades: 2, wins: 1, netR: 1 },
      shorts: { trades: 0, wins: 0, netR: 0 },
      sessions: 3,
      skipped: { maxRangeWidth: 1 },
    });
  });

  it('emits each alert type with stable ids', () => {
    const res = runOrb(day(D, { ...LONG_BREAK, '09:55': [20072, 20300, 19970, 20100] }));
    expect(res.events.map((e) => e.type)).toEqual(['rangeFinalized', 'breakoutConfirmed', 'entry', 'stopHit']);
    expect(res.events[1]).toMatchObject({ direction: 'long', price: 20070 });
    expect(res.events.at(-1)).toMatchObject({ r: -1, tradeId: res.trades[0].id });
    expect(runOrb(day(D, { ...LONG_BREAK, '09:55': [20072, 20300, 19970, 20100] })).events).toEqual(res.events);
  });
});

describe('no lookahead', () => {
  it('appending future bars never changes earlier results', () => {
    const bars = [
      ...day('2026-05-28', { '09:45': [20012, 20075, 20010, 20070], '09:50': [20072, 20260, 20070, 20250] }),
      ...day('2026-05-29', { ...LONG_BREAK, '09:55': [20072, 20080, 19970, 19990] }),
      ...day(D, { '10:00': [19990, 19995, 19960, 19970], '10:05': [19972, 19975, 19700, 19720] }),
    ];
    const full = runOrb(bars, undefined, { stepMs: STEP });
    expect(full.trades.map((t) => t.exitReason)).toEqual(['target', 'stop', 'target']);
    const fullById = new Map(full.events.map((e) => [e.id, e]));
    const fullTrades = new Map(full.trades.map((t) => [t.id, t]));
    for (let k = 1; k <= bars.length; k++) {
      const prefix = bars.slice(0, k);
      // Mid-bar clock: the newest bar is still forming.
      const live = runOrb(prefix, undefined, { nowMs: prefix.at(-1)!.timeMs + STEP / 2, stepMs: STEP });
      for (const e of live.events) expect(fullById.get(e.id)).toEqual(e);
      for (const t of live.trades) {
        if (t.exitReason) expect(fullTrades.get(t.id)).toEqual(t);
      }
    }
  });
});
