// React binding: holds a ChartState, runs tool calls against it, and returns
// props to spread into <VroomChart>.

import { useCallback, useMemo, useRef, useState } from 'react';
import type { Candle, Drawing, DrawingControls, PriceLine, VisibleRange } from '@vroomchart/types';

import { describeChart } from './market';
import type { ChartState } from './state';
import type { HostTool, ToolDefinition, ToolResult } from './tools';
import { runTool, toolDefinitions } from './tools';

export type UseAgentChartOptions = {
  /** The candles passed to the chart. */
  candles: Candle[];
  /** Starting configuration. Read once, on mount. */
  initialState?: ChartState;
  /**
   * The chart's `historyRef`, when it runs with a `drawingStore`. Drawing
   * tools then edit the managed drawings through it, so every agent edit is
   * an undo step and persists. Without it, the hook owns the drawings array
   * and `chartProps` carries it.
   */
  drawingControls?: { current: DrawingControls | null };
  /** Tools your app implements, such as switching symbol or timeframe. */
  hostTools?: readonly HostTool[];
  /** Your own price lines (orders, alerts). Rendered alongside the agent's, never editable by it. */
  priceLines?: PriceLine[];
  /** Forwarded from the chart; the hook also listens to keep the agent's view in sync. */
  onViewportChange?: (startMs: number, endMs: number) => void;
};

/** Props for `<VroomChart>`. Spread them, then pass `candles` and anything else you own. */
export type AgentChartProps = Omit<ChartState, 'drawings'> & {
  priceLines: PriceLine[];
  onViewportChange: (startMs: number, endMs: number) => void;
  drawings?: Drawing[];
  onDrawingComplete?: (d: Drawing) => void;
  onDrawingChange?: (d: Drawing) => void;
  onDrawingDelete?: (id: string) => void;
};

export type AgentChart = {
  /** The current agent-controlled configuration. */
  state: ChartState;
  /** Replace the state directly, e.g. to restore a saved session. */
  setState: (next: ChartState) => void;
  /** Spread into `<VroomChart>`. */
  chartProps: AgentChartProps;
  /** JSON Schema tool definitions; convert with `toOpenAITools` / `toAnthropicTools`. */
  tools: ToolDefinition[];
  /** Run one tool call from the model and apply its result. Never rejects. */
  run: (name: string, args: unknown) => Promise<ToolResult>;
  /** The same summary `get_market_context` starts with, for a system prompt. */
  describe: () => string;
};

/** Diff two drawing lists by id and apply the difference through the chart's controls. */
function syncDrawings(controls: DrawingControls, before: Drawing[], after: Drawing[]) {
  const prev = new Map(before.map((d) => [d.id, d]));
  const next = new Set(after.map((d) => d.id));
  for (const d of before) if (!next.has(d.id)) controls.remove(d.id);
  for (const d of after) {
    const old = prev.get(d.id);
    if (!old) controls.add(d);
    else if (old !== d) controls.update(d);
  }
}

/**
 * Wire a `<VroomChart>` to an AI agent. Send `tools` to your model, pass each
 * tool call it makes to `run`, and return the result string to the model.
 *
 * ```tsx
 * const agent = useAgentChart({ candles });
 * // in your model loop: const { result } = await agent.run(call.name, call.args);
 * return <VroomChart candles={candles} {...agent.chartProps} />;
 * ```
 */
export function useAgentChart(options: UseAgentChartOptions): AgentChart {
  const [state, setStateRaw] = useState<ChartState>(() => options.initialState ?? {});
  // Tool calls arrive back to back from a model loop, faster than renders, so
  // each one reduces over the latest committed state rather than a stale closure.
  const stateRef = useRef(state);
  const optsRef = useRef(options);
  optsRef.current = options;
  const viewportRef = useRef<VisibleRange | null>(null);

  const setState = useCallback((next: ChartState) => {
    stateRef.current = next;
    setStateRaw(next);
  }, []);

  const run = useCallback(
    async (name: string, args: unknown) => {
      const { candles, drawingControls, hostTools } = optsRef.current;
      const controls = drawingControls?.current ?? null;
      const before: ChartState = controls ? { ...stateRef.current, drawings: controls.list() } : stateRef.current;
      const out = await runTool(before, { name, args }, { candles, viewport: viewportRef.current }, hostTools);
      if (out.state !== before) {
        if (controls) {
          syncDrawings(controls, before.drawings ?? [], out.state.drawings ?? []);
          const { drawings: _managed, ...rest } = out.state;
          setState(rest);
        } else {
          setState(out.state);
        }
      }
      return out;
    },
    [setState],
  );

  const describe = useCallback(() => {
    const { candles, drawingControls } = optsRef.current;
    const controls = drawingControls?.current;
    const s = controls ? { ...stateRef.current, drawings: controls.list() } : stateRef.current;
    return describeChart(candles, s, viewportRef.current);
  }, []);

  const onViewportChange = useCallback(
    (startMs: number, endMs: number) => {
      viewportRef.current = { startMs, endMs };
      // Once the agent has set the window, `visibleRange` is controlled, so it
      // has to follow the user's pans or the next render would snap back.
      if (stateRef.current.visibleRange) setState({ ...stateRef.current, visibleRange: { startMs, endMs } });
      optsRef.current.onViewportChange?.(startMs, endMs);
    },
    [setState],
  );

  const onDrawingComplete = useCallback(
    (d: Drawing) => setState({ ...stateRef.current, drawings: [...(stateRef.current.drawings ?? []), d] }),
    [setState],
  );
  const onDrawingChange = useCallback(
    (d: Drawing) =>
      setState({ ...stateRef.current, drawings: (stateRef.current.drawings ?? []).map((x) => (x.id === d.id ? d : x)) }),
    [setState],
  );
  const onDrawingDelete = useCallback(
    (id: string) =>
      setState({ ...stateRef.current, drawings: (stateRef.current.drawings ?? []).filter((x) => x.id !== id) }),
    [setState],
  );

  const managed = options.drawingControls != null;
  const hostPriceLines = options.priceLines;
  const chartProps = useMemo<AgentChartProps>(() => {
    const { drawings, priceLines, ...rest } = state;
    const lines = [...(hostPriceLines ?? []), ...(priceLines ?? [])];
    return managed
      ? { ...rest, priceLines: lines, onViewportChange }
      : {
          ...rest,
          priceLines: lines,
          onViewportChange,
          drawings: drawings ?? [],
          onDrawingComplete,
          onDrawingChange,
          onDrawingDelete,
        };
  }, [state, managed, hostPriceLines, onViewportChange, onDrawingComplete, onDrawingChange, onDrawingDelete]);

  const hostTools = options.hostTools;
  const tools = useMemo(() => toolDefinitions(hostTools), [hostTools]);

  return { state, setState, chartProps, tools, run, describe };
}
