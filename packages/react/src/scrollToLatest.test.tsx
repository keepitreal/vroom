// @vitest-environment jsdom
import { act, cleanup, render } from '@testing-library/react';
import { afterEach, beforeAll, beforeEach, describe, expect, it, vi } from 'vitest';
import type { Candle, ChartControls } from '@vroomchart/types';

const MIN = 60_000;

// Read back history: the newest bar's slot ends at 100 min, the view at 30.
const HISTORY = { startMs: 10 * MIN, endMs: 30 * MIN };
const LATEST = { startMs: 80 * MIN, endMs: 100 * MIN };

const view = { ...HISTORY };
const resetPriceScale = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    resetPriceScale,
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

beforeEach(() => {
  vi.useFakeTimers();
  Object.assign(view, HISTORY);
  resetPriceScale.mockClear();
});

afterEach(() => {
  cleanup();
  vi.useRealTimers();
});

const candles: Candle[] = Array.from({ length: 100 }, (_, i) => ({
  timeMs: i * MIN,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

async function mount(awayFromLatestBars?: number) {
  const chartRef: { current: ChartControls | null } = { current: null };
  const onAway = vi.fn();
  const onViewportChange = vi.fn();
  render(
    <VroomChart
      candles={candles}
      width={800}
      height={400}
      chartRef={chartRef}
      onAwayFromLatestChange={onAway}
      awayFromLatestBars={awayFromLatestBars}
      onViewportChange={onViewportChange}
    />,
  );
  await act(async () => {});
  const frames = (ms: number) =>
    act(() => {
      vi.advanceTimersByTime(ms);
    });
  frames(32);
  return { chartRef, onAway, onViewportChange, frames };
}

describe('scrollToLatest', () => {
  it('reports the view is away from the newest bar, and back once it returns', async () => {
    const { chartRef, onAway, frames } = await mount();
    expect(onAway).toHaveBeenCalledTimes(1);
    expect(onAway).toHaveBeenLastCalledWith(true);

    act(() => chartRef.current!.scrollToLatest());
    frames(1000);
    expect(view).toEqual(LATEST);
    expect(onAway).toHaveBeenCalledTimes(2);
    expect(onAway).toHaveBeenLastCalledWith(false);
  });

  it('glides there, then reports the window it landed on', async () => {
    const { chartRef, onViewportChange, frames } = await mount();
    act(() => chartRef.current!.scrollToLatest({ durationMs: 400, easing: 'linear' }));
    frames(200);
    expect(view.endMs).toBeGreaterThan(HISTORY.endMs);
    expect(view.endMs).toBeLessThan(LATEST.endMs);
    expect(onViewportChange).not.toHaveBeenCalled();

    frames(400);
    expect(view).toEqual(LATEST);
    expect(onViewportChange).toHaveBeenCalledTimes(1);
    expect(onViewportChange).toHaveBeenCalledWith(LATEST.startMs, LATEST.endMs);
  });

  it('returns the price axis to auto-fit', async () => {
    const { chartRef } = await mount();
    const before = resetPriceScale.mock.calls.length;
    act(() => chartRef.current!.scrollToLatest());
    expect(resetPriceScale).toHaveBeenCalledTimes(before + 1);
  });

  it('jumps when not animated', async () => {
    const { chartRef, onViewportChange } = await mount();
    act(() => chartRef.current!.scrollToLatest({ animated: false }));
    expect(view).toEqual(LATEST);
    expect(onViewportChange).toHaveBeenCalledWith(LATEST.startMs, LATEST.endMs);
  });

  it('stops when something else moves the window mid-glide', async () => {
    const { chartRef, onViewportChange, frames } = await mount();
    act(() => chartRef.current!.scrollToLatest({ durationMs: 400 }));
    frames(100);
    const panned = { startMs: view.startMs - 5 * MIN, endMs: view.endMs - 5 * MIN };
    Object.assign(view, panned);
    frames(1000);
    expect(view).toEqual(panned);
    expect(onViewportChange).not.toHaveBeenCalled();
  });

  it('waits for the threshold before reporting away', async () => {
    const { onAway } = await mount(80);
    expect(onAway).not.toHaveBeenCalled();
  });

  it('clears the ref on unmount', async () => {
    const { chartRef } = await mount();
    expect(chartRef.current).not.toBeNull();
    cleanup();
    expect(chartRef.current).toBeNull();
  });
});
