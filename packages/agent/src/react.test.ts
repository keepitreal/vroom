// @vitest-environment jsdom
import { act, renderHook } from '@testing-library/react';
import { describe, expect, it, vi } from 'vitest';
import type { Candle, Drawing, DrawingControls, VroomChartCoreProps } from '@vroomchart/types';

import type { AgentChartProps } from './react';
import { useAgentChart } from './react';

// chartProps must spread straight into <VroomChart>.
export type ChartPropsFit = AgentChartProps extends Partial<VroomChartCoreProps> ? true : never;
const _fits: ChartPropsFit = true;

const candles: Candle[] = Array.from({ length: 30 }, (_, i) => ({
  timeMs: i * 60_000,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

describe('useAgentChart', () => {
  it('applies back-to-back tool calls and exposes them as chart props', async () => {
    const { result } = renderHook(() => useAgentChart({ candles }));
    await act(async () => {
      await result.current.run('set_indicators', { rsi: { lowerBand: 30, upperBand: 70 } });
      await result.current.run('add_moving_average', { maType: 'ema', period: 50 });
    });
    expect(result.current.chartProps.rsi).toMatchObject({ enabled: true, lowerBand: 30, upperBand: 70 });
    expect(result.current.chartProps.movingAverages).toHaveLength(1);
    expect(result.current.tools.some((t) => t.name === 'set_indicators')).toBe(true);
  });

  it('owns drawings in controlled mode, including the user’s', async () => {
    const { result } = renderHook(() => useAgentChart({ candles }));
    const user: Drawing = { id: 'u1', type: 'line', points: [{ timeMs: 0, price: 1 }, { timeMs: 1, price: 2 }] };
    act(() => result.current.chartProps.onDrawingComplete!(user));
    await act(async () => {
      await result.current.run('add_drawing', {
        type: 'line',
        points: [{ barsAgo: 5, price: 100 }, { barsAgo: 0, price: 101 }],
      });
    });
    expect(result.current.chartProps.drawings).toHaveLength(2);
    expect(result.current.chartProps.drawings![0]).toBe(user);
  });

  it('routes drawing edits through the chart’s controls in managed mode', async () => {
    let list: Drawing[] = [];
    const controls: DrawingControls = {
      undo: vi.fn(),
      redo: vi.fn(),
      clearHistory: vi.fn(),
      restyle: vi.fn(),
      add: vi.fn((d: Drawing) => ((list = [...list, d]), true)),
      update: vi.fn(() => true),
      remove: vi.fn((id: string) => ((list = list.filter((d) => d.id !== id)), true)),
      list: () => list,
    };
    const { result } = renderHook(() => useAgentChart({ candles, drawingControls: { current: controls } }));
    await act(async () => {
      await result.current.run('add_drawing', {
        type: 'box',
        points: [{ barsAgo: 5, price: 100 }, { barsAgo: 0, price: 99 }],
      });
      await result.current.run('remove_drawings', { allAgentDrawings: true });
    });
    expect(controls.add).toHaveBeenCalledTimes(1);
    expect(controls.remove).toHaveBeenCalledTimes(1);
    expect(list).toEqual([]);
    expect(result.current.chartProps.drawings).toBeUndefined();
    expect(result.current.state.drawings).toBeUndefined();
  });

  it('merges host price lines with the agent’s and keeps a set viewport following the user', async () => {
    const order = { id: 'order', price: 99 };
    const { result } = renderHook(() => useAgentChart({ candles, priceLines: [order] }));
    await act(async () => {
      await result.current.run('add_price_level', { id: 'r', price: 101 });
      await result.current.run('set_viewport', { lastBars: 10 });
    });
    expect(result.current.chartProps.priceLines.map((l) => l.id)).toEqual(['order', 'r']);
    act(() => result.current.chartProps.onViewportChange(1000, 2000));
    expect(result.current.chartProps.visibleRange).toEqual({ startMs: 1000, endMs: 2000 });
  });
});
