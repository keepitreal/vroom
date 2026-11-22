// @vitest-environment jsdom
import { act, cleanup, render } from '@testing-library/react';
import { afterEach, beforeAll, beforeEach, describe, expect, it, vi } from 'vitest';
import type { Candle, VisibleRange } from '@vroomchart/types';

const MIN = 60_000;
const END = 1_000 * 60 * MIN;

const view = { startMs: 0, endMs: 0 };
const beginStreamMorph = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ ...view }),
    setVisibleRange: ((s: number, e: number) => {
      view.startMs = s;
      view.endMs = e;
    }) as never,
    beginStreamMorph,
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

beforeEach(() => beginStreamMorph.mockClear());
afterEach(cleanup);

function series(count: number, close = 100, endMs = END): Candle[] {
  return Array.from({ length: count }, (_, i) => ({
    timeMs: endMs - (count - i) * MIN,
    open: 100,
    high: 110,
    low: 90,
    close: i === count - 1 ? close : 100,
    volume: 1,
  }));
}

const initial = series(200);
// Pinned to the newest bar, so an uncontrolled chart would slide on append.
const hostRange: VisibleRange = { startMs: END - 80 * MIN, endMs: END };

async function mount() {
  const chart = (c: Candle[], seriesKey = 'BTC') => (
    <VroomChart
      candles={c}
      seriesKey={seriesKey}
      visibleRange={hostRange}
      streamTransition="transform"
      width={800}
      height={400}
    />
  );
  const r = render(chart(initial));
  await act(async () => {});
  return async (c: Candle[], seriesKey?: string) => {
    r.rerender(chart(c, seriesKey));
    await act(async () => {});
  };
}

describe('streamTransition with a controlled visibleRange', () => {
  it('eases a tick to the in-progress bar', async () => {
    const step = await mount();
    await step(series(200, 104));
    expect(beginStreamMorph).toHaveBeenCalledTimes(1);
    expect(view).toEqual(hostRange);
  });

  it('leaves the window to the host when a new bar arrives', async () => {
    const step = await mount();
    await step(series(201, 100, END + MIN));
    expect(beginStreamMorph).not.toHaveBeenCalled();
    expect(view).toEqual(hostRange);
  });

  it('does not morph into a different series', async () => {
    const step = await mount();
    await step(series(200, 104), 'ETH');
    expect(beginStreamMorph).not.toHaveBeenCalled();
  });
});
