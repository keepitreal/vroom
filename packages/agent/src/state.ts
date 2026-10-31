import type { Candle, Drawing, VisibleRange, VroomChartCoreProps } from '@vroomchart/types';

/**
 * The part of a chart an agent can change, as plain JSON: indicators,
 * overlays, display mode, the visible window, the agent's own price levels, and
 * the drawings. Spread it into `<VroomChart>` alongside your `candles`.
 *
 * Candles, callbacks, and theme stay with the host. `priceLines` holds only the
 * levels the agent placed; merge in your own lines (orders, alerts) when you
 * render, so the agent can never move or remove them.
 */
export type ChartState = Pick<
  VroomChartCoreProps,
  | 'chartType'
  | 'priceScaleMode'
  | 'visibleRange'
  | 'rsi'
  | 'macd'
  | 'atr'
  | 'vwap'
  | 'bollingerBands'
  | 'ichimoku'
  | 'fairValueGaps'
  | 'volume'
  | 'movingAverages'
  | 'priceLines'
  | 'drawings'
>;

/** What a tool reads besides the state: the data on screen. */
export type ToolContext = {
  /** The candles the chart is showing, ascending by time. */
  candles: Candle[];
  /**
   * The window the user is looking at, from `onViewportChange`. Optional —
   * without it, tools that care assume the most recent bars.
   */
  viewport?: VisibleRange | null;
};

/** `meta.source` stamped on every drawing an agent creates. */
export const AGENT_SOURCE = 'agent';

/** Whether a drawing was placed by an agent (as opposed to the user). */
export function isAgentDrawing(d: Drawing): boolean {
  return d.meta?.source === AGENT_SOURCE;
}
