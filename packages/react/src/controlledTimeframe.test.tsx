// @vitest-environment jsdom
import { act, cleanup, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle, VisibleRange } from '@vroomchart/types';

const MIN = 60_000;
const HOUR = 60 * MIN;
const END = 1_000 * HOUR;

// A stand-in for the WASM handle whose window actually moves.
const view = { startMs: 0, endMs: 0 };

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ ...view }),
    setVisibleRange: ((s: number, e: number) => {
      view.startMs = s;
      view.endMs = e;
    }) as never,
  };
  return new Proxy(impl, {
    get: (target, key: string) => target[key] ?? (() => null),
  });
}

vi.mock('@vroomchart/core-wasm', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@vroomchart/core-wasm')>();
  return {
    ...actual,
    loadVroom: () => Promise.resolve({ create: () => fakeHandle() }),
  };
});

const { VroomChart } = await import('./VroomChart');

beforeAll(() => {
  HTMLElement.prototype.getBoundingClientRect = () =>
    ({ left: 0, top: 0, right: 800, bottom: 400, width: 800, height: 400, x: 0, y: 0 }) as DOMRect;
  globalThis.ResizeObserver ??= class {
    observe() {}
    unobserve() {}
    disconnect() {}
  } as unknown as typeof ResizeObserver;
});

afterEach(cleanup);

function series(stepMs: number, count: number): Candle[] {
  return Array.from({ length: count }, (_, i) => ({
    timeMs: END - (count - i) * stepMs,
    open: 100,
    high: 101,
    low: 99,
    close: 100,
    volume: 1,
  }));
}

const hourly = series(HOUR, 200);
const fiveMin = series(5 * MIN, 2_400);
// The last 80 hourly slots, ending on the newest candle's slot end.
const hourlyRange: VisibleRange = { startMs: END - 80 * HOUR, endMs: END };

async function mount(onViewportChange: (s: number, e: number) => void) {
  const chart = (c: Candle[], range: VisibleRange) => (
    <VroomChart
      candles={c}
      visibleRange={range}
      onViewportChange={onViewportChange}
      width={800}
      height={400}
      reduceMotion
    />
  );
  const r = render(chart(hourly, hourlyRange));
  await act(async () => {});
  return async (c: Candle[], range: VisibleRange) => {
    r.rerender(chart(c, range));
    await act(async () => {});
  };
}

describe('a controlled visibleRange across a timeframe switch', () => {
  it('re-anchors in slots and reports the new window', async () => {
    const onViewportChange = vi.fn();
    const step = await mount(onViewportChange);
    await step(fiveMin, hourlyRange);
    expect((view.endMs - view.startMs) / (5 * MIN)).toBe(80);
    expect(onViewportChange).toHaveBeenCalledWith(view.startMs, view.endMs);
  });

  it('applies a range the host moves along with the switch', async () => {
    const onViewportChange = vi.fn();
    const step = await mount(onViewportChange);
    const hostRange = { startMs: END - 40 * 5 * MIN, endMs: END };
    await step(fiveMin, hostRange);
    expect(view).toEqual(hostRange);
    expect(onViewportChange).not.toHaveBeenCalled();
  });
});
