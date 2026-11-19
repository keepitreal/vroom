import type { Candle, OrbConfig, OrbResult, OrbSession, OrbTrade, VroomColor } from '@vroomchart/types';

/** A data-space point: epoch ms and price. */
export type AnnotationPoint = { timeMs: number; price: number };

/**
 * Chart-agnostic shapes describing an ORB result. Wrappers translate these to
 * their renderer; nothing here knows about pixels beyond marker offsets.
 */
export type OrbAnnotations = {
  segments: { a: AnnotationPoint; b: AnnotationPoint; color: VroomColor; width: number; dash: 'solid' | 'dotted' | 'dashed' }[];
  boxes: { a: AnnotationPoint; b: AnnotationPoint; fill: VroomColor; border?: VroomColor }[];
  /** `timeMs` is a bar's open time; the marker centers on that bar. `offset` is px, positive = down. */
  markers: { timeMs: number; price: number; shape: 'triangleUp' | 'triangleDown' | 'cross' | 'square' | 'circle'; color: VroomColor; size: number; offset: number }[];
  labels: { timeMs: number; price: number; text: string; color: VroomColor; background?: VroomColor; align: 'start' | 'center' | 'end'; offset: number }[];
};

type ColorKey =
  | 'backgroundColor' | 'midlineColor' | 'openPriceColor' | 'highColor' | 'lowColor'
  | 'longColor' | 'shortColor' | 'entryColor' | 'stopColor' | 'targetColor'
  | 'winColor' | 'lossColor';
type Colors = Record<ColorKey, VroomColor>;

export const ORB_DEFAULT_COLORS: Readonly<Colors> = {
  backgroundColor: '#14ffffff',
  midlineColor: '#e69e9e9e',
  openPriceColor: '#ff9800',
  highColor: '#26a69a',
  lowColor: '#ef5350',
  longColor: '#26a69a',
  shortColor: '#ef5350',
  entryColor: '#2962ff',
  stopColor: '#ef5350',
  targetColor: '#26a69a',
  winColor: '#26a69a',
  lossColor: '#ef5350',
};

const MARKER = 10;
const MARKER_GAP = 9;
const LABEL_BG = '#8c000000';
const LABEL_TEXT = '#ffffff';

const SKIP_TEXT: Record<NonNullable<OrbSession['skipReason']>, string> = {
  incompleteRange: 'ORB skipped: incomplete range',
  maxRangeWidth: 'ORB skipped: range too wide',
  skipDate: 'ORB skipped: date filter',
  skipWeekday: 'ORB skipped: weekday filter',
  trendInsufficientHistory: 'ORB skipped: not enough trend history',
};

/** Index of the bar opening at `timeMs`, by binary search, or -1. */
/** A price distance at any scale: 4 significant digits, trailing zeros dropped (7, 0.3512, 1250). */
function formatPrice(v: number): string {
  return String(Number(v.toPrecision(4)));
}

function barAt(candles: readonly Candle[], timeMs: number): number {
  let lo = 0;
  let hi = candles.length - 1;
  while (lo <= hi) {
    const mid = (lo + hi) >> 1;
    const t = candles[mid].timeMs;
    if (t === timeMs) return mid;
    if (t < timeMs) lo = mid + 1;
    else hi = mid - 1;
  }
  return -1;
}

/**
 * The shapes for `result`: the range box and lines, entry triangles, trade
 * levels, exit markers and status labels, per the `display` settings.
 * `candles` and `stepMs` must be the ones `result` was computed from.
 */
export function orbToAnnotations(
  result: OrbResult,
  candles: readonly Candle[],
  config: OrbConfig | undefined,
  stepMs: number,
): OrbAnnotations {
  const d: Colors = { ...ORB_DEFAULT_COLORS };
  for (const k of Object.keys(ORB_DEFAULT_COLORS) as ColorKey[]) {
    const v = config?.display?.[k];
    if (v != null) d[k] = v;
  }
  const out: OrbAnnotations = { segments: [], boxes: [], markers: [], labels: [] };
  if (candles.length === 0) return out;
  const lastEnd = candles[candles.length - 1].timeMs + stepMs;

  const withRange = result.sessions.filter((s) => s.high != null && s.low != null);
  const shown =
    config?.display?.onlyShowToday ? withRange.slice(-1) : withRange;

  for (const s of shown) {
    const high = s.high!;
    const low = s.low!;
    const boxEnd = s.status === 'forming' ? Math.min(lastEnd, s.rangeEndMs) : (s.finalMs ?? s.rangeEndMs);
    if (config?.display?.showBackgroundFill !== false) {
      out.boxes.push({
        a: { timeMs: s.rangeStartMs, price: high },
        b: { timeMs: boxEnd, price: low },
        fill: d.backgroundColor,
      });
    }
    if (s.status !== 'forming') {
      const from = s.finalMs ?? s.rangeEndMs;
      const line = (price: number, color: VroomColor, dash: 'solid' | 'dashed' | 'dotted') =>
        out.segments.push({ a: { timeMs: from, price }, b: { timeMs: s.flattenMs, price }, color, width: 1.5, dash });
      line(high, d.highColor, 'solid');
      line(low, d.lowColor, 'solid');
      if (config?.display?.showMidline) line((high + low) / 2, d.midlineColor, 'dashed');
      if (config?.display?.showOpenPrice && s.open != null) line(s.open, d.openPriceColor, 'dotted');
    }
    if (s.status === 'skipped' && s.skipReason) {
      out.labels.push({
        timeMs: s.rangeStartMs,
        price: high,
        text: SKIP_TEXT[s.skipReason],
        color: LABEL_TEXT,
        background: LABEL_BG,
        align: 'start',
        offset: -14,
      });
    }
    if (s.pending) {
      out.labels.push({
        timeMs: candles[candles.length - 1].timeMs,
        price: s.pending.level,
        text: `Pending ${s.pending.direction} (candle forming)`,
        color: LABEL_TEXT,
        background: LABEL_BG,
        align: 'end',
        offset: s.pending.direction === 'long' ? -14 : 14,
      });
    }

    if (config?.display?.showSignals !== false) signalMarks(s, candles, d, out);
    for (const t of s.trades) tradeMarks(t, candles, lastEnd, stepMs, config, d, out);
  }
  return out;
}

function signalMarks(s: OrbSession, candles: readonly Candle[], d: Colors, out: OrbAnnotations) {
  for (const sig of s.signals) {
    if (sig.status !== 'entered' && sig.status !== 'awaitingEntry') continue;
    const long = sig.direction === 'long';
    const trade = sig.tradeId ? s.trades.find((t) => t.id === sig.tradeId) : undefined;
    // The bar the trade entered (or would enter) on.
    const timeMs = trade?.entryTimeMs ?? sig.timeMs;
    const i = barAt(candles, timeMs);
    const bar = i >= 0 ? candles[i] : null;
    const price = bar ? (long ? bar.low : bar.high) : sig.close;
    out.markers.push({
      timeMs,
      price,
      shape: long ? 'triangleUp' : 'triangleDown',
      color: long ? d.longColor : d.shortColor,
      size: MARKER,
      offset: long ? MARKER_GAP : -MARKER_GAP,
    });
    const side = long ? 'Long' : 'Short';
    const text =
      sig.status === 'awaitingEntry' || !trade
        ? `${side}: entry next bar`
        : `${side} · risk ${formatPrice(Math.abs(trade.entryPrice - trade.stop))}`;
    out.labels.push({
      timeMs: timeMs,
      price,
      text,
      color: LABEL_TEXT,
      background: LABEL_BG,
      align: 'center',
      offset: long ? MARKER_GAP + MARKER + 10 : -(MARKER_GAP + MARKER + 10),
    });
  }
}

function tradeMarks(
  t: OrbTrade,
  candles: readonly Candle[],
  lastEnd: number,
  stepMs: number,
  config: OrbConfig | undefined,
  d: Colors,
  out: OrbAnnotations,
) {
  const end = t.exitTimeMs ?? lastEnd;
  if (config?.display?.showTradeLevels !== false) {
    const seg = (a: number, b: number, price: number, color: VroomColor, dash: 'solid' | 'dashed') =>
      out.segments.push({ a: { timeMs: a, price }, b: { timeMs: b, price }, color, width: 1.25, dash });
    seg(t.entryTimeMs, end, t.entryPrice, d.entryColor, 'solid');
    const moved = t.partial && t.currentStop !== t.stop;
    seg(t.entryTimeMs, moved ? t.partial!.timeMs : end, t.stop, d.stopColor, 'dashed');
    if (moved) seg(t.partial!.timeMs, end, t.currentStop, d.stopColor, 'dashed');
    if (t.target != null) seg(t.entryTimeMs, end, t.target, d.targetColor, 'dashed');
  }
  if (t.partial) {
    out.markers.push({ timeMs: t.partial.timeMs, price: t.partial.price, shape: 'circle', color: d.winColor, size: 7, offset: 0 });
  }
  if (t.exitTimeMs != null && t.exitPrice != null) {
    // Time exits are stamped at the bar's close; markers center on bar opens.
    const barMs = t.exitReason === 'time' ? t.exitTimeMs - stepMs : t.exitTimeMs;
    const win = (t.r ?? 0) > 0;
    out.markers.push({
      timeMs: barMs,
      price: t.exitPrice,
      shape: t.exitReason === 'time' ? 'square' : 'cross',
      color:
        t.exitReason === 'target' ? d.winColor : t.exitReason === 'stop' ? d.lossColor : win ? d.winColor : d.lossColor,
      size: t.exitReason === 'time' ? 9 : MARKER,
      offset: 0,
    });
    const i = barAt(candles, barMs);
    if (i >= 0 && t.r != null) {
      out.labels.push({
        timeMs: barMs + stepMs,
        price: t.exitPrice,
        text: `${t.r >= 0 ? '+' : ''}${t.r.toFixed(2)}R`,
        color: LABEL_TEXT,
        background: LABEL_BG,
        align: 'start',
        offset: 0,
      });
    }
  }
}