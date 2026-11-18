import type {
  OrbBreakUnit,
  OrbConfig,
  OrbDirectionMode,
  OrbStopMode,
  OrbTargetMode,
} from '@vroomchart/types';

/** `OrbConfig` with every default applied and times parsed to minutes of the day. */
export type ResolvedOrb = {
  timezone: string;
  rangeStartMin: number;
  rangeEndMin: number;
  cutoffMin: number;
  flattenMin: number;

  signalTfMin: number;
  confirmCloses: 1 | 2;
  breakDistance: number;
  breakUnit: OrbBreakUnit;
  directionMode: OrbDirectionMode;
  maxLongSignals: number;
  maxShortSignals: number;
  maxTradesPerDay: number;
  stopAfterLoss: boolean;

  maxRangeWidthPct: number | null;
  skipDates: Set<string>;
  skipWeekdays: Set<number>;
  trend: { enabled: boolean; period: number; dailyCloses?: { timeMs: number; close: number }[] };

  stopMode: OrbStopMode;
  stopPoints: number;
  stopBufferTicks: number;
  targetMode: OrbTargetMode;
  targetR: number;
  rangeMult: number;
  partial: { enabled: boolean; atR: number; fraction: number; breakeven: boolean };
  riskPerTrade: number;
  pointValue: number;
  tickSize: number;
  allowReversal: boolean;
};

const PRESET_MIN = { '5m': 5, '15m': 15, '30m': 30, '60m': 60 } as const;

/** Minutes after midnight for `'HH:MM'`, or `fallback` when it doesn't parse. */
export function parseTimeOfDay(s: string | undefined, fallback: number): number {
  if (!s) return fallback;
  const m = /^(\d{1,2}):(\d{2})$/.exec(s.trim());
  if (!m) return fallback;
  const h = Number(m[1]);
  const min = Number(m[2]);
  if (h > 23 || min > 59) return fallback;
  return h * 60 + min;
}

const positive = (v: number | undefined, fallback: number) =>
  v != null && Number.isFinite(v) && v > 0 ? v : fallback;
const nonNegative = (v: number | undefined, fallback: number) =>
  v != null && Number.isFinite(v) && v >= 0 ? v : fallback;
const count = (v: number | undefined, fallback: number) =>
  v != null && Number.isFinite(v) && v >= 0 ? Math.floor(v) : fallback;

export function resolveOrb(cfg: OrbConfig | undefined): ResolvedOrb {
  const s = cfg?.session ?? {};
  const c = cfg?.confirmation ?? {};
  const f = cfg?.filters ?? {};
  const r = cfg?.risk ?? {};

  const rangeStartMin = parseTimeOfDay(s.rangeStart, 9 * 60 + 30);
  const preset = s.rangePreset ?? '15m';
  const rangeEndMin =
    preset === 'custom'
      ? parseTimeOfDay(s.rangeEnd, rangeStartMin + 15)
      : rangeStartMin + PRESET_MIN[preset];

  const width = f.maxRangeWidthPct;
  const maxRangeWidthPct =
    width === false || width === 0 ? null : positive(width === undefined ? undefined : width, 0.85);

  return {
    timezone: s.timezone || 'America/New_York',
    rangeStartMin,
    rangeEndMin: Math.max(rangeEndMin, rangeStartMin + 1),
    cutoffMin: parseTimeOfDay(s.entryCutoff, 12 * 60),
    flattenMin: parseTimeOfDay(s.flattenTime, 15 * 60 + 55),

    signalTfMin: Math.round(positive(c.signalTimeframe, 5)),
    confirmCloses: c.confirmCloses === 2 ? 2 : 1,
    breakDistance: nonNegative(c.minBreakDistance, 0),
    breakUnit: c.minBreakUnit ?? 'ticks',
    directionMode: c.directionMode ?? 'both',
    maxLongSignals: count(c.maxBullishSignals, 1),
    maxShortSignals: count(c.maxBearishSignals, 1),
    maxTradesPerDay: count(c.maxTradesPerDay, 1),
    stopAfterLoss: c.stopAfterLoss ?? true,

    maxRangeWidthPct,
    skipDates: new Set(f.skipDates ?? []),
    skipWeekdays: new Set(f.skipWeekdays ?? []),
    trend: {
      enabled: f.trendFilter?.enabled ?? false,
      period: Math.max(1, Math.floor(positive(f.trendFilter?.period, 200))),
      dailyCloses: f.trendFilter?.dailyCloses,
    },

    stopMode: r.stopMode ?? 'oppositeSide',
    stopPoints: positive(r.stopPoints, 10),
    stopBufferTicks: nonNegative(r.stopBufferTicks, 0),
    targetMode: r.targetMode ?? 'rMultiple',
    targetR: positive(r.targetR, 2),
    rangeMult: positive(r.rangeMult, 1),
    partial: {
      enabled: r.partial?.enabled ?? false,
      atR: positive(r.partial?.atR, 1),
      fraction: Math.min(1, positive(r.partial?.pct, 50) / 100),
      breakeven: r.partial?.moveStopToBreakeven ?? true,
    },
    riskPerTrade: positive(r.riskPerTrade, 250),
    pointValue: positive(r.pointValue, 2),
    tickSize: positive(r.tickSize, 0.25),
    allowReversal: r.allowReversal ?? false,
  };
}
