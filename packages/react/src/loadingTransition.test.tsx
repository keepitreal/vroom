// @vitest-environment jsdom
import { act, cleanup, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle } from '@vroomchart/types';

const MIN = 60_000;
const HOUR = 60 * MIN;
const END = 1_000 * HOUR;

// A stand-in for the WASM handle whose window actually moves, so the test can
// read back the view the data effect leaves behind.
const view = { startMs: 0, endMs: 0 };
const resetView = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ ...view }),
    getVisiblePriceEnvelope: () => ({ low: 99, high: 101 }),
    setVisibleRange: ((s: number, e: number) => {
      view.startMs = s;
      view.endMs = e;
    }) as never,
    resetView: resetView as never,
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

afterEach(() => {
  cleanup();
  resetView.mockClear();
});

function series(stepMs: number, count: number, close = 100): Candle[] {
  return Array.from({ length: count }, (_, i) => ({
    timeMs: END - (count - i) * stepMs,
    open: close,
    high: close + 1,
    low: close - 1,
    close,
    volume: 1,
  }));
}

const hourly = series(HOUR, 200);
const fiveMin = series(5 * MIN, 2_400);

async function mountFramed(candles: Candle[], stepMs: number, seriesKey?: string) {
  const chart = (c: Candle[], loading = false, key = seriesKey) => (
    <VroomChart candles={c} loading={loading} seriesKey={key} width={800} height={400} reduceMotion />
  );
  const r = render(chart(candles));
  await act(async () => {});
  // Frame the last 80 slots, ending on the newest candle's slot end.
  const lastMs = candles[candles.length - 1].timeMs;
  view.endMs = lastMs + stepMs;
  view.startMs = view.endMs - 80 * stepMs;
  const step = async (c: Candle[], loading = false, key = seriesKey) => {
    r.rerender(chart(c, loading, key));
    await act(async () => {});
  };
  return step;
}

const slots = (stepMs: number) => (view.endMs - view.startMs) / stepMs;

describe('a switch through the loading line', () => {
  it('re-anchors a timeframe switch, keeping the visible slot count', async () => {
    const step = await mountFramed(hourly, HOUR);
    await step([], true);
    await step(fiveMin);
    expect(slots(5 * MIN)).toBe(80);
    expect(view.endMs).toBe(fiveMin[fiveMin.length - 1].timeMs + 5 * MIN);
    expect(resetView).not.toHaveBeenCalled();
  });

  it('resets the view for a different series', async () => {
    const step = await mountFramed(hourly, HOUR, 'BTC');
    await step([], true, 'SOL');
    await step(series(HOUR, 200, 2), false, 'SOL');
    expect(resetView).toHaveBeenCalledTimes(1);
  });

  it('leaves the window alone when the same series comes back', async () => {
    const step = await mountFramed(hourly, HOUR);
    const before = { ...view };
    await step([], true);
    await step(series(HOUR, 200));
    expect(view).toEqual(before);
    expect(resetView).not.toHaveBeenCalled();
  });
});
