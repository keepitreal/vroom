import type { Candle } from '@vroomchart/types';

import { classifyTransition, type DataTransition } from './dataTransitions';

/** What the core holds, or is about to, as far as classifying a push goes. */
export type DataState = {
  candles: Candle[];
  seriesKey?: string;
  transitionKey?: string | number;
};

/**
 * Classify a candles push onto a core that already holds `prev`.
 *
 * A new `transitionKey` on the same series is the host declaring a timeframe
 * switch, so it morphs whatever the data heuristics make of it: the step can be
 * unchanged, or the data can look like a different asset. Otherwise an
 * explicit visibleRange overrides any auto behavior, so the change is treated
 * like a stream and the range application wins. Without either, the data
 * heuristics decide.
 */
export function classifyDataChange(
  prev: DataState,
  next: DataState,
  explicitRange: boolean,
): DataTransition {
  const seriesKeyChanged = next.seriesKey !== prev.seriesKey;
  if (!seriesKeyChanged && next.transitionKey !== prev.transitionKey) return 'timeframe';
  if (explicitRange) return 'stream';
  return classifyTransition(prev.candles, next.candles, seriesKeyChanged);
}
