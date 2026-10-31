import { describe, expect, it } from 'vitest';
import type { Candle, Drawing } from '@vroomchart/types';

import { applyAction, runTool, toAnthropicTools, toOpenAITools, toolDefinitions, defineHostTool } from './tools';
import type { ChartState } from './state';
import { z } from 'zod';

const STEP = 15 * 60_000;
const candles: Candle[] = Array.from({ length: 50 }, (_, i) => ({
  timeMs: 1_700_000_000_000 + i * STEP,
  open: 100 + i,
  high: 101 + i,
  low: 99 + i,
  close: 100.5 + i,
  volume: 10,
}));
const ctx = { candles };
const last = candles[candles.length - 1]!;

describe('set_indicators', () => {
  it('turns RSI on with 30/70 bands', () => {
    const out = applyAction({}, { name: 'set_indicators', args: { rsi: { lowerBand: 30, upperBand: 70 } } }, ctx);
    expect(out.ok).toBe(true);
    expect(out.state.rsi).toEqual({ lowerBand: 30, upperBand: 70, enabled: true });
  });

  it('merges over the current config and can switch an indicator off', () => {
    const state: ChartState = { rsi: { enabled: true, period: 21 } };
    const out = applyAction(state, { name: 'set_indicators', args: { rsi: { enabled: false } } }, ctx);
    expect(out.state.rsi).toEqual({ enabled: false, period: 21 });
  });

  it('rejects inverted bands without changing state', () => {
    const state: ChartState = {};
    const out = applyAction(state, { name: 'set_indicators', args: { rsi: { lowerBand: 80 } } }, ctx);
    expect(out.ok).toBe(false);
    expect(out.state).toBe(state);
    expect(out.result).toMatch(/lowerBand must be below upperBand/);
  });

  it('explains schema errors, including unknown keys', () => {
    const out = applyAction({}, { name: 'set_indicators', args: { rsi: { period: 0 }, stochastic: {} } }, ctx);
    expect(out.ok).toBe(false);
    expect(out.result).toMatch(/invalid arguments/);
    expect(out.result).toMatch(/period/);
    expect(out.result).toMatch(/stochastic/);
  });
});

describe('moving averages', () => {
  it('adds with a default id, replaces on the same id, and removes', () => {
    let s = applyAction({}, { name: 'add_moving_average', args: { maType: 'ema', period: 200 } }, ctx).state;
    expect(s.movingAverages).toEqual([{ maType: 'ema', period: 200, id: 'ema-200' }]);
    s = applyAction(s, { name: 'add_moving_average', args: { maType: 'ema', period: 200, color: '#ff0000' } }, ctx).state;
    expect(s.movingAverages).toHaveLength(1);
    expect(s.movingAverages![0]!.color).toBe('#ff0000');
    s = applyAction(s, { name: 'remove_moving_average', args: { id: 'ema-200' } }, ctx).state;
    expect(s.movingAverages).toEqual([]);
  });
});

describe('drawings', () => {
  it('resolves barsAgo anchors, including into the future, and tags the drawing', () => {
    const out = applyAction(
      {},
      {
        name: 'add_drawing',
        args: {
          type: 'box',
          points: [
            { barsAgo: 10, price: 120 },
            { barsAgo: -5, price: 118 },
          ],
          label: 'Support',
        },
      },
      ctx,
    );
    expect(out.ok).toBe(true);
    const d = out.state.drawings![0]!;
    expect(d.type).toBe('box');
    expect(d.points[0]!.timeMs).toBe(candles[candles.length - 11]!.timeMs);
    expect(d.points[1]!.timeMs).toBe(last.timeMs + 5 * STEP);
    expect(d.meta).toEqual({ source: 'agent' });
    expect(d.label).toBe('Support');
    expect(out.result).toContain(d.id);
  });

  it('rejects a line with three points and an out-of-range barsAgo', () => {
    const three = applyAction(
      {},
      { name: 'add_drawing', args: { type: 'line', points: [1, 2, 3].map((n) => ({ barsAgo: n, price: 1 })) } },
      ctx,
    );
    expect(three.result).toMatch(/exactly 2 points/);
    const far = applyAction(
      {},
      { name: 'add_drawing', args: { type: 'line', points: [{ barsAgo: 999, price: 1 }, { barsAgo: 0, price: 1 }] } },
      ctx,
    );
    expect(far.result).toMatch(/only 50 candles/);
  });

  it('requires exactly one of timeMs or barsAgo', () => {
    const out = applyAction(
      {},
      { name: 'add_drawing', args: { type: 'line', points: [{ price: 1 }, { timeMs: 1, barsAgo: 0, price: 1 }] } },
      ctx,
    );
    expect(out.ok).toBe(false);
    expect(out.result).toMatch(/exactly one of timeMs or barsAgo/);
  });

  const userLine: Drawing = { id: 'u1', type: 'line', points: [{ timeMs: 1, price: 1 }, { timeMs: 2, price: 2 }] };
  const lockedLine: Drawing = { ...userLine, id: 'u2', locked: true };
  const agentLine: Drawing = { ...userLine, id: 'a1', meta: { source: 'agent' } };

  it('removes only its own drawings with allAgentDrawings', () => {
    const out = applyAction(
      { drawings: [userLine, agentLine] },
      { name: 'remove_drawings', args: { allAgentDrawings: true } },
      ctx,
    );
    expect(out.state.drawings).toEqual([userLine]);
  });

  it('keeps drawings the user locked', () => {
    const out = applyAction(
      { drawings: [userLine, lockedLine] },
      { name: 'remove_drawings', args: { ids: ['u1', 'u2'] } },
      ctx,
    );
    expect(out.state.drawings).toEqual([lockedLine]);
    expect(out.result).toMatch(/Kept 1 locked/);
    const upd = applyAction({ drawings: [lockedLine] }, { name: 'update_drawing', args: { id: 'u2', color: '#ff0000' } }, ctx);
    expect(upd.ok).toBe(false);
  });

  it('updates style and keeps geometry and meta', () => {
    const out = applyAction({ drawings: [agentLine] }, { name: 'update_drawing', args: { id: 'a1', color: '#00ff00' } }, ctx);
    expect(out.state.drawings![0]).toEqual({ ...agentLine, color: '#00ff00' });
  });
});

describe('price levels and view', () => {
  it('adds, moves by id, and removes levels', () => {
    let s = applyAction({}, { name: 'add_price_level', args: { id: 'r1', price: 150, text: 'Resistance' } }, ctx).state;
    s = applyAction(s, { name: 'add_price_level', args: { id: 'r1', price: 151 } }, ctx).state;
    expect(s.priceLines).toEqual([{ id: 'r1', price: 151 }]);
    s = applyAction(s, { name: 'remove_price_levels', args: { all: true } }, ctx).state;
    expect(s.priceLines).toEqual([]);
  });

  it('frames the last N bars', () => {
    const out = applyAction({}, { name: 'set_viewport', args: { lastBars: 20 } }, ctx);
    expect(out.state.visibleRange).toEqual({ startMs: candles[30]!.timeMs, endMs: last.timeMs });
  });

  it('switches to a log line chart', () => {
    const out = applyAction({}, { name: 'set_chart_display', args: { chartType: 'line', priceScaleMode: 'log' } }, ctx);
    expect(out.state).toEqual({ chartType: 'line', priceScaleMode: 'log' });
  });

  it('reads state and market context without changing state', () => {
    const state: ChartState = { rsi: { enabled: true } };
    const read = applyAction(state, { name: 'get_chart_state', args: {} }, ctx);
    expect(read.state).toBe(state);
    expect(JSON.parse(read.result)).toEqual(state);
    const market = applyAction(state, { name: 'get_market_context', args: {} }, ctx);
    expect(market.state).toBe(state);
    expect(market.result).toMatch(/Interval: 15m/);
    expect(market.result).toMatch(/RSI\(14\)/);
  });
});

describe('dispatch', () => {
  it('reports an unknown tool', () => {
    const out = applyAction({}, { name: 'draw_fib', args: {} }, ctx);
    expect(out.ok).toBe(false);
    expect(out.result).toMatch(/unknown tool draw_fib/);
  });

  it('runs host tools and turns their errors into results', async () => {
    const setTimeframe = defineHostTool({
      name: 'set_timeframe',
      description: 'Switch timeframe.',
      schema: z.strictObject({ interval: z.enum(['1m', '15m']) }),
      execute: ({ interval }) => {
        if (interval === '1m') throw new Error('not available');
        return `Switched to ${interval}.`;
      },
    });
    const ok = await runTool({}, { name: 'set_timeframe', args: { interval: '15m' } }, ctx, [setTimeframe]);
    expect(ok).toMatchObject({ ok: true, result: 'Switched to 15m.' });
    const bad = await runTool({}, { name: 'set_timeframe', args: { interval: '1m' } }, ctx, [setTimeframe]);
    expect(bad).toMatchObject({ ok: false, result: 'Error: set_timeframe failed: not available' });
    const built = await runTool({}, { name: 'set_chart_display', args: { chartType: 'line' } }, ctx, [setTimeframe]);
    expect(built.state).toEqual({ chartType: 'line' });
  });
});

describe('tool definitions', () => {
  const defs = toolDefinitions();

  it('give every tool an object-rooted JSON Schema', () => {
    expect(defs.length).toBeGreaterThan(10);
    for (const d of defs) {
      expect(d.inputSchema.type, d.name).toBe('object');
      expect(d.inputSchema.$schema).toBeUndefined();
      expect(d.description.length).toBeGreaterThan(20);
    }
  });

  it('convert to the OpenAI and Anthropic formats', () => {
    expect(toOpenAITools(defs)[0]).toMatchObject({ type: 'function', function: { name: defs[0]!.name } });
    expect(toAnthropicTools(defs)[0]).toMatchObject({ name: defs[0]!.name, input_schema: defs[0]!.inputSchema });
  });
});
