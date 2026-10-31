// Market context for agents: a compact text description of what's on the
// chart, and support/resistance levels derived from swing points. Both exist so
// the model reasons from real prices instead of inventing them.

import type { Candle, VisibleRange } from '@vroomchart/types';

import type { ChartState } from './state';
import { isAgentDrawing } from './state';

/** Candle period in ms: the median of the first few intervals. Null below two candles. */
export function inferStepMs(candles: Candle[]): number | null {
  if (candles.length < 2) return null;
  const k = Math.min(candles.length - 1, 8);
  const diffs: number[] = [];
  for (let i = 0; i < k; i++) diffs.push(candles[i + 1]!.timeMs - candles[i]!.timeMs);
  diffs.sort((a, b) => a - b);
  const median = diffs[Math.floor(diffs.length / 2)]!;
  return median > 0 ? median : null;
}

/** A step like `15m`, `4h`, `1d`. */
export function formatInterval(stepMs: number): string {
  const units: [number, string][] = [
    [86_400_000, 'd'],
    [3_600_000, 'h'],
    [60_000, 'm'],
    [1000, 's'],
  ];
  for (const [ms, unit] of units) {
    if (stepMs >= ms && stepMs % ms === 0) return `${stepMs / ms}${unit}`;
  }
  return `${stepMs}ms`;
}

/** The candles inside `range` (inclusive), or the last `fallbackBars` without one. */
export function visibleCandles(
  candles: Candle[],
  range: VisibleRange | null | undefined,
  fallbackBars = 100,
): Candle[] {
  if (range && range.endMs > range.startMs) {
    const inView = candles.filter((c) => c.timeMs >= range.startMs && c.timeMs <= range.endMs);
    if (inView.length > 0) return inView;
  }
  return candles.slice(-fallbackBars);
}

// ── Indicator readings ───────────────────────────────────────────────────────
// Latest values only, matching the chart's definitions closely enough for an
// agent to talk about them ("RSI is 72, overbought").

function sma(values: number[], period: number): number | null {
  if (values.length < period || period < 1) return null;
  let sum = 0;
  for (let i = values.length - period; i < values.length; i++) sum += values[i]!;
  return sum / period;
}

function emaSeries(values: number[], period: number): number[] {
  if (values.length < period || period < 1) return [];
  const k = 2 / (period + 1);
  let prev = values.slice(0, period).reduce((a, b) => a + b, 0) / period;
  const out = [prev];
  for (let i = period; i < values.length; i++) {
    prev = values[i]! * k + prev * (1 - k);
    out.push(prev);
  }
  return out;
}

/** Wilder's RSI of the closes, latest value. */
export function latestRSI(candles: Candle[], period = 14): number | null {
  if (candles.length <= period) return null;
  let gain = 0;
  let loss = 0;
  for (let i = 1; i <= period; i++) {
    const d = candles[i]!.close - candles[i - 1]!.close;
    if (d > 0) gain += d;
    else loss -= d;
  }
  gain /= period;
  loss /= period;
  for (let i = period + 1; i < candles.length; i++) {
    const d = candles[i]!.close - candles[i - 1]!.close;
    gain = (gain * (period - 1) + Math.max(d, 0)) / period;
    loss = (loss * (period - 1) + Math.max(-d, 0)) / period;
  }
  if (loss === 0) return gain === 0 ? 50 : 100;
  return 100 - 100 / (1 + gain / loss);
}

/** Wilder's ATR, latest value. */
export function latestATR(candles: Candle[], period = 14): number | null {
  if (candles.length <= period) return null;
  const tr: number[] = [];
  for (let i = 1; i < candles.length; i++) {
    const c = candles[i]!;
    const prevClose = candles[i - 1]!.close;
    tr.push(Math.max(c.high - c.low, Math.abs(c.high - prevClose), Math.abs(c.low - prevClose)));
  }
  let atr = tr.slice(0, period).reduce((a, b) => a + b, 0) / period;
  for (let i = period; i < tr.length; i++) atr = (atr * (period - 1) + tr[i]!) / period;
  return atr;
}

/** MACD line, signal, and histogram (EMA legs), latest values. */
export function latestMACD(
  candles: Candle[],
  fast = 12,
  slow = 26,
  signal = 9,
): { macd: number; signal: number; histogram: number } | null {
  const closes = candles.map((c) => c.close);
  const f = emaSeries(closes, fast);
  const s = emaSeries(closes, slow);
  if (s.length === 0) return null;
  const offset = slow - fast;
  const line = s.map((v, i) => f[i + offset]! - v);
  const sig = emaSeries(line, signal);
  if (sig.length === 0) return null;
  const m = line[line.length - 1]!;
  const g = sig[sig.length - 1]!;
  return { macd: m, signal: g, histogram: m - g };
}

/** Latest SMA or EMA of the closes. */
export function latestMA(candles: Candle[], maType: 'sma' | 'ema', period: number): number | null {
  const closes = candles.map((c) => c.close);
  if (maType === 'sma') return sma(closes, period);
  const e = emaSeries(closes, period);
  return e.length > 0 ? e[e.length - 1]! : null;
}

// ── Support and resistance ───────────────────────────────────────────────────

/** A horizontal level price has turned at more than once. */
export type SwingLevel = {
  /** The level, as the mean of the clustered swing prices. */
  price: number;
  /** Above the last close is resistance; at or below it, support. */
  kind: 'support' | 'resistance';
  /** How many swing highs/lows landed on this level. */
  touches: number;
  /** Time of the earliest swing on the level, epoch ms. */
  firstTouchMs: number;
  /** Time of the most recent swing on the level, epoch ms. */
  lastTouchMs: number;
};

export type SwingLevelOptions = {
  /** Bars on each side a swing high/low must dominate. Default 3. */
  strength?: number;
  /**
   * How close two swings must be to count as the same level, in price.
   * Default half the 14-bar ATR, so it scales with the instrument.
   */
  tolerance?: number;
  /** Most levels to return, strongest first. Default 6. */
  maxLevels?: number;
  /** Ignore levels with fewer touches. Default 2. */
  minTouches?: number;
};

/**
 * Support and resistance levels from swing points: pivot highs and lows that
 * dominate `strength` bars on each side, clustered by price within
 * `tolerance`. Sorted by touch count, then recency.
 */
export function findSwingLevels(candles: Candle[], opts: SwingLevelOptions = {}): SwingLevel[] {
  const strength = Math.max(1, Math.floor(opts.strength ?? 3));
  const maxLevels = opts.maxLevels ?? 6;
  const minTouches = opts.minTouches ?? 2;
  if (candles.length < strength * 2 + 1) return [];
  const tolerance =
    opts.tolerance ?? (latestATR(candles, Math.min(14, candles.length - 1)) ?? 0) * 0.5;

  const swings: { price: number; timeMs: number }[] = [];
  for (let i = strength; i < candles.length - strength; i++) {
    const c = candles[i]!;
    let isHigh = true;
    let isLow = true;
    for (let j = i - strength; j <= i + strength; j++) {
      if (j === i) continue;
      if (candles[j]!.high >= c.high) isHigh = false;
      if (candles[j]!.low <= c.low) isLow = false;
    }
    if (isHigh) swings.push({ price: c.high, timeMs: c.timeMs });
    if (isLow) swings.push({ price: c.low, timeMs: c.timeMs });
  }

  swings.sort((a, b) => a.price - b.price);
  const clusters: { price: number; timeMs: number }[][] = [];
  for (const s of swings) {
    const last = clusters[clusters.length - 1];
    if (last && s.price - last[0]!.price <= tolerance) last.push(s);
    else clusters.push([s]);
  }

  const lastClose = candles[candles.length - 1]!.close;
  return clusters
    .filter((c) => c.length >= minTouches)
    .map((c): SwingLevel => {
      const price = c.reduce((a, s) => a + s.price, 0) / c.length;
      const times = c.map((s) => s.timeMs);
      return {
        price,
        kind: price > lastClose ? 'resistance' : 'support',
        touches: c.length,
        firstTouchMs: Math.min(...times),
        lastTouchMs: Math.max(...times),
      };
    })
    .sort((a, b) => b.touches - a.touches || b.lastTouchMs - a.lastTouchMs)
    .slice(0, maxLevels);
}

// ── Description ──────────────────────────────────────────────────────────────

/** Price with significant digits that suit its magnitude. */
export function formatPrice(p: number): string {
  const abs = Math.abs(p);
  const digits = abs >= 1000 ? 2 : abs >= 1 ? 4 : 6;
  return Number(p.toFixed(digits)).toString();
}

const iso = (ms: number) => new Date(ms).toISOString().replace('.000Z', 'Z');

/**
 * A compact, model-readable summary of the chart: interval, the window in
 * view, price action across it, current indicator readings, and what's drawn.
 * Meant to go straight into a tool result or system prompt.
 */
export function describeChart(
  candles: Candle[],
  state: ChartState = {},
  viewport?: VisibleRange | null,
): string {
  if (candles.length === 0) return 'The chart has no candles loaded.';
  const lines: string[] = [];
  const step = inferStepMs(candles);
  const last = candles[candles.length - 1]!;
  const view = visibleCandles(candles, viewport ?? state.visibleRange);
  const first = view[0]!;
  const end = view[view.length - 1]!;

  lines.push(
    `Interval: ${step ? formatInterval(step) : 'unknown'}. ${candles.length} candles loaded, ` +
      `${iso(candles[0]!.timeMs)} to ${iso(last.timeMs)}.`,
  );
  let hi = first;
  let lo = first;
  for (const c of view) {
    if (c.high > hi.high) hi = c;
    if (c.low < lo.low) lo = c;
  }
  const change = ((end.close - first.open) / first.open) * 100;
  lines.push(
    `In view: ${view.length} bars, ${iso(first.timeMs)} to ${iso(end.timeMs)}. ` +
      `Open ${formatPrice(first.open)}, close ${formatPrice(end.close)} (${change >= 0 ? '+' : ''}${change.toFixed(2)}%). ` +
      `High ${formatPrice(hi.high)} at ${iso(hi.timeMs)}, low ${formatPrice(lo.low)} at ${iso(lo.timeMs)}.`,
  );
  lines.push(`Last candle ${iso(last.timeMs)}: O ${formatPrice(last.open)} H ${formatPrice(last.high)} L ${formatPrice(last.low)} C ${formatPrice(last.close)}.`);

  const readings: string[] = [];
  if (state.rsi?.enabled) {
    const v = latestRSI(candles, state.rsi.period ?? 14);
    if (v != null) {
      const up = state.rsi.upperBand ?? 70;
      const down = state.rsi.lowerBand ?? 30;
      const zone = v >= up ? ' (overbought)' : v <= down ? ' (oversold)' : '';
      readings.push(`RSI(${state.rsi.period ?? 14}) ${v.toFixed(1)}${zone}, bands ${down}/${up}`);
    }
  }
  if (state.macd?.enabled) {
    const m = latestMACD(candles, state.macd.fast ?? 12, state.macd.slow ?? 26, state.macd.signal ?? 9);
    if (m) {
      readings.push(
        `MACD ${formatPrice(m.macd)}, signal ${formatPrice(m.signal)}, histogram ${formatPrice(m.histogram)}`,
      );
    }
  }
  if (state.atr?.enabled) {
    const v = latestATR(candles, state.atr.period ?? 14);
    if (v != null) readings.push(`ATR(${state.atr.period ?? 14}) ${formatPrice(v)}`);
  }
  for (const ma of state.movingAverages ?? []) {
    const v = latestMA(candles, ma.maType, ma.period);
    if (v != null) {
      readings.push(`${ma.maType.toUpperCase()}(${ma.period}) ${formatPrice(v)}${ma.id ? ` [id ${ma.id}]` : ''}`);
    }
  }
  const overlays = (
    [
      ['VWAP', state.vwap?.enabled],
      ['Bollinger Bands', state.bollingerBands?.enabled],
      ['Ichimoku', state.ichimoku?.enabled],
      ['Fair value gaps', state.fairValueGaps?.enabled],
    ] as const
  )
    .filter(([, on]) => on)
    .map(([name]) => name);
  if (readings.length > 0) lines.push(`Indicators: ${readings.join('; ')}.`);
  if (overlays.length > 0) lines.push(`Also shown: ${overlays.join(', ')}.`);
  if (state.volume?.enabled === false) lines.push('Volume bars are hidden.');

  const levels = state.priceLines ?? [];
  if (levels.length > 0) {
    lines.push(
      `Agent price levels: ${levels
        .map((l) => `${formatPrice(l.price)}${l.text ? ` "${l.text}"` : ''} [id ${l.id}]`)
        .join('; ')}.`,
    );
  }
  const drawings = state.drawings ?? [];
  if (drawings.length > 0) {
    const mine = drawings.filter(isAgentDrawing);
    const desc = mine.map((d) => `${d.type}${d.label ? ` "${d.label}"` : ''} [id ${d.id}]`);
    lines.push(
      `Drawings: ${drawings.length} total, ${drawings.length - mine.length} by the user` +
        (desc.length > 0 ? `, yours: ${desc.join('; ')}.` : '.'),
    );
  }
  return lines.join('\n');
}
