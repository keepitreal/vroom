// Classifies how a new `candles` prop relates to the previous one so the chart
// can react appropriately: leave the viewport alone for streaming updates,
// re-anchor the time window for a timeframe switch, or fully reset the view
// for a different asset. Pure functions, no React — see useChartCore for the
// orchestration.

import type { Candle, VisibleRange } from '@vroomchart/types';

export type DataTransition = 'initial' | 'stream' | 'timeframe' | 'reset';

// A step change below this ratio is treated as the same timeframe. Real steps
// are exact integer ms; the tolerance only absorbs rounding/DST quirks (the
// smallest real timeframe jump, 1m -> 2m, is 100% apart).
const STEP_TOLERANCE = 0.01;

// Same-asset check for a timeframe switch: both series end "now", so their
// last closes must be close. No asset moves 25% between two consecutive prop
// pushes; distinct assets within 25% of each other are what `seriesKey` is for.
const MAX_SAME_ASSET_CLOSE_RATIO = 1.25;

// A coarser bucketing can shift the final bar's open by up to one coarse bar;
// allow that plus an in-flight bar when checking the two series end together.
const MAX_END_DRIFT_STEPS = 3;

// Streaming pushes may batch a few bars (e.g. a throttled background tab), but
// a jump of more than this many steps means the data was re-fetched elsewhere.
const MAX_STREAM_ADVANCE_STEPS = 5;

/**
 * The candle period in ms, inferred as the median of the first few intervals
 * (robust to a single gap). Null when there are fewer than two candles.
 */
export function inferStepMs(candles: Candle[]): number | null {
  if (candles.length < 2) return null;
  const k = Math.min(candles.length - 1, 8);
  const diffs: number[] = [];
  for (let i = 0; i < k; i++) diffs.push(candles[i + 1].timeMs - candles[i].timeMs);
  diffs.sort((a, b) => a - b);
  const median = diffs[Math.floor(diffs.length / 2)];
  return median > 0 ? median : null;
}

// Index of the candle whose timeMs exactly equals `t`, or -1. Binary search over
// the ascending-by-time series, so it tolerates interior gaps (missing bars from
// downtime / illiquid periods) — unlike a uniform-grid index computed from the
// step, which assumes a hole-free grid.
function indexByTime(candles: Candle[], t: number): number {
  let lo = 0;
  let hi = candles.length - 1;
  while (lo <= hi) {
    const mid = (lo + hi) >>> 1;
    const v = candles[mid].timeMs;
    if (v === t) return mid;
    if (v < t) lo = mid + 1;
    else hi = mid - 1;
  }
  return -1;
}

/**
 * Classify a candles-prop change. `prev` is the previously rendered array
 * (null on first render); `seriesKeyChanged` forces `reset` regardless of the
 * data (the explicit escape hatch).
 *
 * Constraint: detection compares two immutable snapshots. An array mutated in
 * place (same reference) never reaches this code — React props must change
 * identity to re-render.
 */
export function classifyTransition(
  prev: Candle[] | null,
  next: Candle[],
  seriesKeyChanged: boolean,
): DataTransition {
  if (!prev || prev.length === 0) return 'initial';
  if (next.length === 0) return 'stream'; // nothing to reframe against
  if (seriesKeyChanged) return 'reset';

  const prevStep = inferStepMs(prev);
  const nextStep = inferStepMs(next);
  if (prevStep == null || nextStep == null) return 'reset'; // too little data to reason

  const prevLast = prev[prev.length - 1];
  const nextLast = next[next.length - 1];

  if (Math.abs(nextStep - prevStep) <= prevStep * STEP_TOLERANCE) {
    // Same step: streaming iff prev's last bar still appears in next (covers
    // append, update-last, and rolling buffers that drop old bars from the
    // front) and the series only advanced by a few bars. Locate that bar by
    // timestamp, not by a step-derived index — real series have interior gaps
    // (downtime / illiquid periods), so a uniform-grid index would miss it and
    // misread a harmless in-place tick as a reset.
    // Time alignment alone isn't enough: two assets on the same exchange share
    // the bar grid, so the bar at the shared timestamp must also be (nearly) the
    // same bar — update-last moves the close, but never by the same-asset ratio.
    const idx = indexByTime(next, prevLast.timeMs);
    const aligned = idx >= 0;
    const sharedBarRatio =
      aligned && next[idx].close > 0 && prevLast.close > 0
        ? Math.max(next[idx].close / prevLast.close, prevLast.close / next[idx].close)
        : Infinity;
    const advanced =
      nextLast.timeMs >= prevLast.timeMs &&
      nextLast.timeMs - prevLast.timeMs <= MAX_STREAM_ADVANCE_STEPS * nextStep;
    return sharedBarRatio <= MAX_SAME_ASSET_CLOSE_RATIO && advanced ? 'stream' : 'reset';
  }

  // Step changed: a timeframe switch iff it still looks like the same asset —
  // last closes near each other and both series ending around the same time.
  const closeRatio =
    prevLast.close > 0 && nextLast.close > 0
      ? Math.max(nextLast.close / prevLast.close, prevLast.close / nextLast.close)
      : Infinity;
  const prevEnd = prevLast.timeMs + prevStep;
  const nextEnd = nextLast.timeMs + nextStep;
  const endsTogether =
    Math.abs(nextEnd - prevEnd) <= MAX_END_DRIFT_STEPS * Math.max(prevStep, nextStep);
  return closeRatio <= MAX_SAME_ASSET_CLOSE_RATIO && endsTogether ? 'timeframe' : 'reset';
}

/**
 * The visible window to apply after a timeframe switch so each candle keeps
 * the exact pixel width it had before: the visible slot count is preserved and
 * the right edge re-anchors on the newest candle (any future-gap overshoot is
 * carried over in slots, clamped to the core's 3/4-window cap). The new start
 * may precede the first candle — that gap is intentional, width wins.
 */
export function timeframeWindow(
  oldWindow: VisibleRange,
  oldStepMs: number,
  oldLastMs: number,
  newStepMs: number,
  newLastMs: number,
): VisibleRange {
  const slots = (oldWindow.endMs - oldWindow.startMs) / oldStepMs;
  const offsetRaw = (oldWindow.endMs - oldLastMs) / oldStepMs;
  const offsetSlots = Math.min(Math.max(offsetRaw, 0), slots * 0.75);
  const endMs = Math.round(newLastMs + offsetSlots * newStepMs);
  return { startMs: Math.round(endMs - slots * newStepMs), endMs };
}
