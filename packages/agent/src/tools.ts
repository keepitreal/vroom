// The chart's agent tools: a name, a description written for the model, an
// argument schema, and a pure run function over ChartState. applyAction is the
// one entry point that validates and dispatches; nothing here mutates.

import { z } from 'zod';
import type { DrawPoint, Drawing, PriceLine } from '@vroomchart/types';

import { describeChart, findSwingLevels, formatPrice, inferStepMs } from './market';
import * as s from './schemas';
import type { ChartState, ToolContext } from './state';
import { AGENT_SOURCE, isAgentDrawing } from './state';

/** The outcome of one tool call. `result` is meant to be returned to the model verbatim. */
export type ToolResult = {
  /** The state after the call. Unchanged (same reference) on error or for read-only tools. */
  state: ChartState;
  /** A short, model-readable summary of what happened, or what went wrong. */
  result: string;
  /** False when the call was rejected; `result` then explains why. */
  ok: boolean;
};

/** A tool call as a model emits it. */
export type ToolCall = { name: string; args: unknown };

type RunOutput = { state?: ChartState; result: string };

type ChartTool<S extends z.ZodType = z.ZodType> = {
  name: string;
  description: string;
  schema: S;
  run: (args: z.infer<S>, state: ChartState, ctx: ToolContext) => RunOutput;
};

class ToolError extends Error {}

const tool = <S extends z.ZodType>(t: ChartTool<S>): ChartTool => t as unknown as ChartTool;

const newId = (prefix: string) => `${prefix}-${crypto.randomUUID().slice(0, 8)}`;

function resolveAnchor(a: z.infer<typeof s.anchorSchema>, ctx: ToolContext): DrawPoint {
  if (a.timeMs != null) return { timeMs: a.timeMs, price: a.price };
  const { candles } = ctx;
  const n = a.barsAgo!;
  if (candles.length === 0) throw new ToolError('No candles are loaded, so barsAgo has nothing to count from. Use timeMs.');
  const lastIndex = candles.length - 1;
  if (n >= 0) {
    if (n > lastIndex) throw new ToolError(`barsAgo ${n} is out of range: only ${candles.length} candles are loaded.`);
    return { timeMs: candles[lastIndex - n]!.timeMs, price: a.price };
  }
  const step = inferStepMs(candles);
  if (step == null) throw new ToolError('Need at least two candles to reach into the future with a negative barsAgo.');
  return { timeMs: candles[lastIndex]!.timeMs - n * step, price: a.price };
}

function buildDrawing(
  id: string,
  type: Drawing['type'],
  points: DrawPoint[],
  style: { color?: string; width?: number; fill?: string; label?: string },
  meta: Drawing['meta'],
): Drawing {
  if ((type === 'line' || type === 'box') && points.length !== 2) {
    throw new ToolError(`A ${type} takes exactly 2 points; got ${points.length}.`);
  }
  if (style.fill != null && type !== 'box') throw new ToolError('fill only applies to a box.');
  const common = {
    id,
    ...(style.color != null ? { color: style.color } : {}),
    ...(style.width != null ? { width: style.width } : {}),
    ...(style.label != null ? { label: style.label } : {}),
    ...(meta != null ? { meta } : {}),
  };
  if (type === 'line') return { ...common, type, points: [points[0]!, points[1]!] };
  if (type === 'box') {
    return { ...common, type, points: [points[0]!, points[1]!], ...(style.fill != null ? { fill: style.fill } : {}) };
  }
  return { ...common, type, points };
}

const describeDrawing = (d: Drawing) =>
  `${d.type}${d.label ? ` "${d.label}"` : ''} [id ${d.id}] at ${d.points
    .map((p) => `(${new Date(p.timeMs).toISOString()}, ${formatPrice(p.price)})`)
    .join(' → ')}`;

type IndicatorKey = keyof z.infer<typeof s.setIndicatorsSchema>;

const INDICATOR_TOOLS: ChartTool[] = [
  tool({
    name: 'set_indicators',
    description:
      'Show, hide, or configure indicators. Pass only the indicators to change; settings merge over what is already set, ' +
      'and passing an indicator turns it on unless you set enabled: false. ' +
      'Example: {"rsi": {"period": 14, "lowerBand": 30, "upperBand": 70}} adds RSI with 30/70 bands.',
    schema: s.setIndicatorsSchema,
    run(args, state) {
      const next: ChartState = { ...state };
      const summary: string[] = [];
      for (const key of Object.keys(args) as IndicatorKey[]) {
        const patch = args[key];
        if (!patch) continue;
        const merged = { ...state[key], ...patch, enabled: patch.enabled ?? true } as Record<string, unknown>;
        if (key === 'rsi' && (Number(merged.lowerBand ?? 30) >= Number(merged.upperBand ?? 70))) {
          throw new ToolError('RSI lowerBand must be below upperBand.');
        }
        if (key === 'macd' && Number(merged.fast ?? 12) >= Number(merged.slow ?? 26)) {
          throw new ToolError('MACD fast must be shorter than slow.');
        }
        (next as Record<string, unknown>)[key] = merged;
        summary.push(merged.enabled ? `${key} on ${JSON.stringify(patch)}` : `${key} off`);
      }
      if (summary.length === 0) throw new ToolError('Pass at least one indicator to change.');
      return { state: next, result: `Updated: ${summary.join('; ')}.` };
    },
  }),
  tool({
    name: 'add_moving_average',
    description:
      'Add an SMA or EMA line to the price pane, or replace one by reusing its id. ' +
      'Example: {"maType": "ema", "period": 200, "color": "#f5a623"}.',
    schema: s.movingAverageSchema,
    run(args, state) {
      const id = args.id ?? `${args.maType}-${args.period}`;
      const ma = { ...args, id };
      const list = state.movingAverages ?? [];
      const exists = list.some((m) => m.id === id);
      const next = exists ? list.map((m) => (m.id === id ? ma : m)) : [...list, ma];
      return {
        state: { ...state, movingAverages: next },
        result: `${exists ? 'Replaced' : 'Added'} ${args.maType.toUpperCase()}(${args.period}) [id ${id}].`,
      };
    },
  }),
  tool({
    name: 'remove_moving_average',
    description: 'Remove a moving-average line by id.',
    schema: s.removeByIdSchema,
    run(args, state) {
      const list = state.movingAverages ?? [];
      if (!list.some((m) => m.id === args.id)) {
        const ids = list.map((m) => m.id).filter(Boolean);
        throw new ToolError(`No moving average has id ${args.id}.${ids.length ? ` Ids: ${ids.join(', ')}.` : ''}`);
      }
      return { state: { ...state, movingAverages: list.filter((m) => m.id !== args.id) }, result: `Removed ${args.id}.` };
    },
  }),
];

const DRAWING_TOOLS: ChartTool[] = [
  tool({
    name: 'add_drawing',
    description:
      'Draw a trendline, box, or arrowed path on the price pane. Points are a price plus either timeMs or barsAgo ' +
      '(0 = latest candle, negative = into the future). Use a box spanning a price band for a support/resistance zone, ' +
      'e.g. {"type": "box", "points": [{"barsAgo": 120, "price": 64100}, {"barsAgo": -20, "price": 63800}], "label": "Support"}. ' +
      'Prices must come from the data (see get_market_context), not guesses.',
    schema: s.addDrawingSchema,
    run(args, state, ctx) {
      const points = args.points.map((p) => resolveAnchor(p, ctx));
      const d = buildDrawing(newId(args.type), args.type, points, args, { source: AGENT_SOURCE });
      return { state: { ...state, drawings: [...(state.drawings ?? []), d] }, result: `Added ${describeDrawing(d)}.` };
    },
  }),
  tool({
    name: 'update_drawing',
    description: 'Move or restyle a drawing by id. Omitted fields keep their current values. Locked drawings cannot be changed.',
    schema: s.updateDrawingSchema,
    run(args, state, ctx) {
      const list = state.drawings ?? [];
      const prev = list.find((d) => d.id === args.id);
      if (!prev) throw new ToolError(`No drawing has id ${args.id}.`);
      if (prev.locked) throw new ToolError(`Drawing ${args.id} is locked by the user and can't be changed.`);
      const points = args.points ? args.points.map((p) => resolveAnchor(p, ctx)) : prev.points;
      const d = buildDrawing(
        prev.id,
        prev.type,
        points,
        {
          color: args.color ?? (prev.color as string | undefined),
          width: args.width ?? prev.width,
          fill: args.fill ?? (prev.type === 'box' ? (prev.fill as string | undefined) : undefined),
          label: args.label ?? prev.label,
        },
        prev.meta,
      );
      return { state: { ...state, drawings: list.map((x) => (x.id === d.id ? d : x)) }, result: `Updated ${describeDrawing(d)}.` };
    },
  }),
  tool({
    name: 'remove_drawings',
    description:
      'Remove drawings by id, or every drawing you created with allAgentDrawings: true. Drawings the user locked are kept.',
    schema: s.removeDrawingsSchema,
    run(args, state) {
      const list = state.drawings ?? [];
      if (!args.ids?.length && !args.allAgentDrawings) throw new ToolError('Pass ids or allAgentDrawings: true.');
      const ids = new Set(args.ids ?? []);
      const missing = [...ids].filter((id) => !list.some((d) => d.id === id));
      if (missing.length) throw new ToolError(`No drawing has id ${missing.join(', ')}.`);
      const target = (d: Drawing) => ids.has(d.id) || (args.allAgentDrawings === true && isAgentDrawing(d));
      const locked = list.filter((d) => target(d) && d.locked);
      const removed = list.filter((d) => target(d) && !d.locked);
      return {
        state: { ...state, drawings: list.filter((d) => !removed.includes(d)) },
        result:
          `Removed ${removed.length} drawing${removed.length === 1 ? '' : 's'}.` +
          (locked.length ? ` Kept ${locked.length} locked by the user.` : ''),
      };
    },
  }),
  tool({
    name: 'add_price_level',
    description:
      'Draw a horizontal line across the chart at a price, with an optional label. Reusing an id moves that level. ' +
      'Good for key support/resistance levels, targets, or stops.',
    schema: s.addPriceLevelSchema,
    run(args, state) {
      const id = args.id ?? newId('level');
      const line: PriceLine = { ...args, id };
      const list = state.priceLines ?? [];
      const exists = list.some((l) => l.id === id);
      return {
        state: { ...state, priceLines: exists ? list.map((l) => (l.id === id ? line : l)) : [...list, line] },
        result: `${exists ? 'Moved' : 'Added'} level ${formatPrice(args.price)}${args.text ? ` "${args.text}"` : ''} [id ${id}].`,
      };
    },
  }),
  tool({
    name: 'remove_price_levels',
    description: 'Remove price levels you placed, by id or all at once.',
    schema: s.removePriceLevelsSchema,
    run(args, state) {
      const list = state.priceLines ?? [];
      if (!args.ids?.length && !args.all) throw new ToolError('Pass ids or all: true.');
      const missing = (args.ids ?? []).filter((id) => !list.some((l) => l.id === id));
      if (missing.length) throw new ToolError(`No price level has id ${missing.join(', ')}.`);
      const next = args.all ? [] : list.filter((l) => !args.ids!.includes(l.id));
      return { state: { ...state, priceLines: next }, result: `Removed ${list.length - next.length} level(s).` };
    },
  }),
];

const VIEW_TOOLS: ChartTool[] = [
  tool({
    name: 'set_viewport',
    description: 'Change the visible time window: the most recent N candles with lastBars, or an explicit startMs/endMs.',
    schema: s.setViewportSchema,
    run(args, state, ctx) {
      let startMs: number;
      let endMs: number;
      if (args.lastBars != null) {
        const c = ctx.candles;
        if (c.length < 2) throw new ToolError('Not enough candles loaded to frame.');
        const n = Math.min(args.lastBars, c.length);
        startMs = c[c.length - n]!.timeMs;
        endMs = c[c.length - 1]!.timeMs;
      } else {
        if (args.startMs == null || args.endMs == null) throw new ToolError('Pass both startMs and endMs.');
        if (args.endMs <= args.startMs) throw new ToolError('endMs must be after startMs.');
        startMs = args.startMs;
        endMs = args.endMs;
      }
      return {
        state: { ...state, visibleRange: { startMs, endMs } },
        result: `Showing ${new Date(startMs).toISOString()} to ${new Date(endMs).toISOString()}.`,
      };
    },
  }),
  tool({
    name: 'set_chart_display',
    description: 'Switch between candles and a line chart, and between a linear and log price scale.',
    schema: s.setChartDisplaySchema,
    run(args, state) {
      if (args.chartType == null && args.priceScaleMode == null) throw new ToolError('Pass chartType or priceScaleMode.');
      return { state: { ...state, ...args }, result: `Display set: ${JSON.stringify(args)}.` };
    },
  }),
  tool({
    name: 'get_chart_state',
    description: 'Read the chart configuration as JSON: indicators, moving averages, price levels, drawings, and display settings.',
    schema: s.emptySchema,
    run: (_args, state) => ({ result: JSON.stringify(state) }),
  }),
  tool({
    name: 'get_market_context',
    description:
      'Describe what the chart shows (interval, window in view, recent price action, indicator readings, drawings) and list ' +
      'support/resistance levels found from swing highs and lows. Call this before drawing anything price-based.',
    schema: s.getMarketContextSchema,
    run(args, state, ctx) {
      const text = describeChart(ctx.candles, state, ctx.viewport);
      const levels = findSwingLevels(ctx.candles, { strength: args.strength, maxLevels: args.maxLevels });
      const levelText = levels.length
        ? levels
            .map(
              (l) =>
                `${l.kind} ${formatPrice(l.price)} (${l.touches} touches, last ${new Date(l.lastTouchMs).toISOString()})`,
            )
            .join('\n')
        : 'No repeated swing levels found.';
      return { result: `${text}\n\nSwing levels:\n${levelText}` };
    },
  }),
];

/** Every built-in chart tool. */
export const chartTools: readonly ChartTool[] = [...INDICATOR_TOOLS, ...DRAWING_TOOLS, ...VIEW_TOOLS];

/**
 * Run one built-in tool call against `state`. Validates the arguments, never
 * throws, and never mutates `state`; a rejected call returns the same state with
 * `ok: false` and an explanation in `result`.
 */
export function applyAction(state: ChartState, call: ToolCall, ctx: ToolContext): ToolResult {
  const t = chartTools.find((x) => x.name === call.name);
  if (!t) {
    return { state, ok: false, result: `Error: unknown tool ${call.name}. Available: ${chartTools.map((x) => x.name).join(', ')}.` };
  }
  const parsed = t.schema.safeParse(call.args ?? {});
  if (!parsed.success) {
    return { state, ok: false, result: `Error: invalid arguments for ${t.name}:\n${z.prettifyError(parsed.error)}` };
  }
  try {
    const out = t.run(parsed.data, state, ctx);
    return { state: out.state ?? state, ok: true, result: out.result };
  } catch (e) {
    if (e instanceof ToolError) return { state, ok: false, result: `Error: ${e.message}` };
    throw e;
  }
}

// ── Host tools ───────────────────────────────────────────────────────────────

/**
 * A tool the host app implements, for things the chart doesn't own: switching
 * the symbol or timeframe, placing an order, fetching more history.
 */
export type HostTool<S extends z.ZodType = z.ZodType> = {
  name: string;
  description: string;
  schema: S;
  execute: (args: z.infer<S>, ctx: ToolContext & { state: ChartState }) => string | Promise<string>;
};

/** Type-safe constructor for a {@link HostTool}. */
export function defineHostTool<S extends z.ZodType>(t: HostTool<S>): HostTool {
  return t as unknown as HostTool;
}

/**
 * Run a tool call, built-in or host. Host tools run first by name, so a host can
 * override a built-in. Like {@link applyAction}, rejected calls resolve with
 * `ok: false` rather than throwing; errors thrown by a host tool become results.
 */
export async function runTool(
  state: ChartState,
  call: ToolCall,
  ctx: ToolContext,
  hostTools: readonly HostTool[] = [],
): Promise<ToolResult> {
  const host = hostTools.find((t) => t.name === call.name);
  if (!host) return applyAction(state, call, ctx);
  const parsed = host.schema.safeParse(call.args ?? {});
  if (!parsed.success) {
    return { state, ok: false, result: `Error: invalid arguments for ${host.name}:\n${z.prettifyError(parsed.error)}` };
  }
  try {
    return { state, ok: true, result: await host.execute(parsed.data, { ...ctx, state }) };
  } catch (e) {
    return { state, ok: false, result: `Error: ${host.name} failed: ${e instanceof Error ? e.message : String(e)}` };
  }
}

// ── Provider formats ─────────────────────────────────────────────────────────

/** A provider-neutral tool definition. Also the shape MCP's `tools/list` returns. */
export type ToolDefinition = {
  name: string;
  description: string;
  inputSchema: Record<string, unknown>;
};

function jsonSchema(schema: z.ZodType): Record<string, unknown> {
  const { $schema: _drop, ...rest } = z.toJSONSchema(schema, { io: 'input' }) as Record<string, unknown>;
  return rest;
}

/** JSON Schema definitions for the built-in tools plus any host tools. */
export function toolDefinitions(hostTools: readonly HostTool[] = []): ToolDefinition[] {
  const hostNames = new Set(hostTools.map((t) => t.name));
  return [...chartTools.filter((t) => !hostNames.has(t.name)), ...hostTools].map((t) => ({
    name: t.name,
    description: t.description,
    inputSchema: jsonSchema(t.schema),
  }));
}

/** Definitions in the OpenAI Chat Completions `tools` format. */
export function toOpenAITools(defs: readonly ToolDefinition[]) {
  return defs.map((d) => ({
    type: 'function' as const,
    function: { name: d.name, description: d.description, parameters: d.inputSchema },
  }));
}

/** Definitions in the Anthropic Messages API `tools` format. */
export function toAnthropicTools(defs: readonly ToolDefinition[]) {
  return defs.map((d) => ({ name: d.name, description: d.description, input_schema: d.inputSchema }));
}
