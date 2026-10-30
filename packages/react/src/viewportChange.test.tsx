// @vitest-environment jsdom
import { act, cleanup, fireEvent, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle } from '@vroomchart/types';

// A stand-in for the WASM handle whose viewport mutators actually move the
// window, so onViewportChange has something real to report.
const view = { startMs: 0, endMs: 600_000 };

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ ...view }),
    translate: (dx: number) => {
      view.startMs -= dx * 1000;
      view.endMs -= dx * 1000;
    },
    pan: (dx: number) => {
      view.startMs -= dx * 1000;
      view.endMs -= dx * 1000;
    },
    zoom: (fx: number) => {
      const mid = (view.startMs + view.endMs) / 2;
      const half = (view.endMs - view.startMs) / 2 / fx;
      view.startMs = Math.round(mid - half);
      view.endMs = Math.round(mid + half);
    },
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
  HTMLElement.prototype.setPointerCapture = () => {};
  HTMLElement.prototype.releasePointerCapture = () => {};
  HTMLElement.prototype.hasPointerCapture = () => false;
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
  vi.useRealTimers();
  view.startMs = 0;
  view.endMs = 600_000;
});

const candles: Candle[] = Array.from({ length: 10 }, (_, i) => ({
  timeMs: i * 60_000,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

async function mount() {
  const onViewportChange = vi.fn();
  const r = render(
    <VroomChart candles={candles} width={800} height={400} onViewportChange={onViewportChange} />,
  );
  await act(async () => {});
  const el = r.container.querySelector('canvas')!.parentElement!;
  const pointer = (type: 'down' | 'move' | 'up', x: number, y: number) =>
    act(() => {
      const init = { pointerId: 1, pointerType: 'mouse', button: 0, clientX: x, clientY: y };
      if (type === 'down') fireEvent.pointerDown(el, init);
      else if (type === 'move') fireEvent.pointerMove(el, init);
      else fireEvent.pointerUp(el, init);
    });
  const wheel = (deltaY: number) =>
    act(() => {
      fireEvent.wheel(el, { deltaY, clientX: 300, clientY: 200 });
    });
  return { pointer, wheel, onViewportChange };
}

describe('onViewportChange', () => {
  it('reports the real window once a drag pan is released', async () => {
    const { pointer, onViewportChange } = await mount();
    pointer('down', 300, 200);
    pointer('move', 350, 200);
    expect(onViewportChange).not.toHaveBeenCalled();
    pointer('up', 350, 200);
    expect(onViewportChange).toHaveBeenCalledTimes(1);
    expect(onViewportChange).toHaveBeenCalledWith(view.startMs, view.endMs);
    expect(view.startMs).not.toBe(0);
  });

  it('stays quiet for a click that did not move the window', async () => {
    const { pointer, onViewportChange } = await mount();
    pointer('down', 300, 200);
    pointer('up', 300, 200);
    expect(onViewportChange).not.toHaveBeenCalled();
  });

  it('reports once after wheel input settles', async () => {
    vi.useFakeTimers();
    const { wheel, onViewportChange } = await mount();
    wheel(-100);
    wheel(-100);
    wheel(-100);
    expect(onViewportChange).not.toHaveBeenCalled();
    act(() => {
      vi.advanceTimersByTime(200);
    });
    expect(onViewportChange).toHaveBeenCalledTimes(1);
    expect(onViewportChange).toHaveBeenCalledWith(view.startMs, view.endMs);
  });
});
