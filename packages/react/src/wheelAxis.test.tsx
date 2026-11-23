// @vitest-environment jsdom
import { act, cleanup, fireEvent, render } from '@testing-library/react';
import { afterEach, beforeAll, beforeEach, describe, expect, it, vi } from 'vitest';
import type { Candle } from '@vroomchart/types';

// 800×400 chart: a 60px price axis on the right, a 24px time axis at the
// bottom, and a 100px indicator pane above it.
const W = 800;
const H = 400;
const metrics = { yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 100 };

const zoom = vi.fn();
const pan = vi.fn();
const scalePriceAxis = vi.fn();
const scaleIndicatorAxis = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => metrics,
    getVisibleRange: () => ({ startMs: 0, endMs: 600_000 }),
    zoom,
    pan,
    scalePriceAxis,
    scaleIndicatorAxis,
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
    ({ left: 0, top: 0, right: W, bottom: H, width: W, height: H, x: 0, y: 0 }) as DOMRect;
  globalThis.ResizeObserver ??= class {
    observe() {}
    unobserve() {}
    disconnect() {}
  } as unknown as typeof ResizeObserver;
});

beforeEach(() => {
  for (const f of [zoom, pan, scalePriceAxis, scaleIndicatorAxis]) f.mockClear();
});
afterEach(cleanup);

const candles: Candle[] = Array.from({ length: 10 }, (_, i) => ({
  timeMs: i * 60_000,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

// Points inside each region.
const PLOT = { x: 300, y: 100 };
const PRICE_AXIS = { x: W - 20, y: 100 };
const INDICATOR_AXIS = { x: W - 20, y: H - 24 - 50 };
const TIME_AXIS = { x: 300, y: H - 10 };

async function mount() {
  const r = render(<VroomChart candles={candles} width={W} height={H} />);
  await act(async () => {});
  const el = r.container.querySelector('canvas')!.parentElement!;
  return (at: { x: number; y: number }, init: WheelEventInit) =>
    act(() => {
      fireEvent.wheel(el, { clientX: at.x, clientY: at.y, ...init });
    });
}

describe('wheel over the price axis', () => {
  it('scales price only: wheel down widens, wheel up tightens', async () => {
    const wheel = await mount();
    wheel(PRICE_AXIS, { deltaY: 100 });
    wheel(PRICE_AXIS, { deltaY: -100 });
    expect(scalePriceAxis).toHaveBeenCalledTimes(2);
    expect(scalePriceAxis.mock.calls[0][0]).toBeGreaterThan(0);
    expect(scalePriceAxis.mock.calls[1][0]).toBeLessThan(0);
    expect(zoom).not.toHaveBeenCalled();
    expect(pan).not.toHaveBeenCalled();
  });

  it('keeps a pinch (ctrl+wheel) on the price axis too', async () => {
    const wheel = await mount();
    wheel(PRICE_AXIS, { deltaY: 40, ctrlKey: true });
    expect(scalePriceAxis).toHaveBeenCalledTimes(1);
    expect(zoom).not.toHaveBeenCalled();
  });
});

describe('wheel over an indicator y-axis', () => {
  it('scales that pane only', async () => {
    const wheel = await mount();
    wheel(INDICATOR_AXIS, { deltaY: 100 });
    expect(scaleIndicatorAxis).toHaveBeenCalledTimes(1);
    expect(scaleIndicatorAxis.mock.calls[0][0]).toBe(INDICATOR_AXIS.y);
    expect(scaleIndicatorAxis.mock.calls[0][1]).toBeGreaterThan(0);
    expect(scalePriceAxis).not.toHaveBeenCalled();
    expect(zoom).not.toHaveBeenCalled();
  });
});

describe('wheel over the time axis', () => {
  it('zooms time only', async () => {
    const wheel = await mount();
    wheel(TIME_AXIS, { deltaY: 100 });
    expect(zoom).toHaveBeenCalledTimes(1);
    expect(zoom.mock.calls[0][1]).toBe(1);
    expect(scalePriceAxis).not.toHaveBeenCalled();
  });

  it('keeps a pinch (ctrl+wheel) off the price axis', async () => {
    const wheel = await mount();
    wheel(TIME_AXIS, { deltaY: 40, ctrlKey: true });
    expect(zoom).toHaveBeenCalledTimes(1);
    expect(zoom.mock.calls[0][1]).toBe(1);
  });

  it('pans on a horizontal scroll', async () => {
    const wheel = await mount();
    wheel(TIME_AXIS, { deltaX: 50, deltaY: 0 });
    expect(pan).toHaveBeenCalledWith(-50, 0);
    expect(zoom).not.toHaveBeenCalled();
  });
});

describe('wheel over the plot', () => {
  it('still zooms time, and a pinch still zooms both axes', async () => {
    const wheel = await mount();
    wheel(PLOT, { deltaY: 100 });
    expect(zoom.mock.calls[0][1]).toBe(1);
    wheel(PLOT, { deltaY: 40, ctrlKey: true });
    expect(zoom.mock.calls[1][1]).toBe(zoom.mock.calls[1][0]);
    expect(scalePriceAxis).not.toHaveBeenCalled();
  });
});
