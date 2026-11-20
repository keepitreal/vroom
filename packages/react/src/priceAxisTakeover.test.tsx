// @vitest-environment jsdom
import { act, cleanup, fireEvent, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle, VroomTheme } from '@vroomchart/types';

const scalePriceAxis = vi.fn();
const resetPriceScale = vi.fn();
const zoom = vi.fn();
const translate = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ startMs: 0, endMs: 600_000 }),
    scalePriceAxis: scalePriceAxis as never,
    resetPriceScale: resetPriceScale as never,
    zoom: zoom as never,
    translate: translate as never,
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
  vi.clearAllMocks();
});

const candles: Candle[] = Array.from({ length: 10 }, (_, i) => ({
  timeMs: i * 60_000,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

// The fake strip is the right-hand 60px of an 800px chart.
const AXIS_X = 780;
const PLOT_X = 300;

type Props = { autoscaleY?: boolean; manualPriceScale?: boolean; theme?: VroomTheme };

async function mount(props: Props) {
  const ui = (p: Props) => <VroomChart candles={candles} width={800} height={400} {...p} />;
  const r = render(ui(props));
  await act(async () => {});
  // The data push that mounts the chart re-fits the price scale itself.
  resetPriceScale.mockClear();
  const el = r.container.querySelector('canvas')!.parentElement!;
  const pointer = (type: 'down' | 'move' | 'up', x: number, y: number) =>
    act(() => {
      const init = { pointerId: 1, pointerType: 'mouse', button: 0, clientX: x, clientY: y };
      if (type === 'down') fireEvent.pointerDown(el, init);
      else if (type === 'move') fireEvent.pointerMove(el, init);
      else fireEvent.pointerUp(el, init);
    });
  const drag = (x: number) => {
    pointer('down', x, 200);
    pointer('move', x, 240);
    pointer('up', x, 240);
  };
  const doubleClick = (x: number) =>
    act(() => {
      fireEvent.doubleClick(el, { clientX: x, clientY: 200 });
    });
  const pinchWheel = () =>
    act(() => {
      fireEvent.wheel(el, { deltaY: -100, ctrlKey: true, clientX: PLOT_X, clientY: 200 });
    });
  const update = async (p: Props) => {
    r.rerender(ui(p));
    await act(async () => {});
  };
  return { drag, doubleClick, pinchWheel, update };
}

describe('price axis under autoscaleY', () => {
  it('leaves the price range alone without manualPriceScale', async () => {
    const { drag, doubleClick, pinchWheel } = await mount({ autoscaleY: true });
    drag(AXIS_X);
    expect(scalePriceAxis).not.toHaveBeenCalled();
    doubleClick(AXIS_X);
    expect(resetPriceScale).not.toHaveBeenCalled();
    pinchWheel();
    expect(zoom).toHaveBeenCalledTimes(1);
    expect(zoom.mock.calls[0]![1]).toBe(1);
  });

  it('lets a drag on the price axis take the scale over with manualPriceScale', async () => {
    const { drag, pinchWheel } = await mount({ autoscaleY: true, manualPriceScale: true });
    drag(AXIS_X);
    expect(scalePriceAxis).toHaveBeenCalledWith(40);
    // The zoom's y goes to the core, which drops it until the scale is taken over.
    pinchWheel();
    expect(zoom.mock.calls[0]![1]).not.toBe(1);
  });

  it('never takes the scale over from a drag on the plot', async () => {
    const { drag } = await mount({ autoscaleY: true, manualPriceScale: true });
    drag(PLOT_X);
    expect(scalePriceAxis).not.toHaveBeenCalled();
    expect(translate).toHaveBeenCalled();
  });

  it('hands the scale back on a double-click on the price axis only', async () => {
    const { doubleClick } = await mount({ autoscaleY: true, manualPriceScale: true });
    doubleClick(PLOT_X);
    expect(resetPriceScale).not.toHaveBeenCalled();
    doubleClick(AXIS_X);
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
  });

  it('hands the scale back when the price axis is hidden, and locks it again', async () => {
    const { drag, doubleClick, update } = await mount({ autoscaleY: true, manualPriceScale: true });
    await update({ autoscaleY: true, manualPriceScale: true, theme: { showYAxis: false } });
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
    drag(AXIS_X);
    doubleClick(AXIS_X);
    expect(scalePriceAxis).not.toHaveBeenCalled();
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
  });

  it('hands the scale back when manualPriceScale is turned off', async () => {
    const { update } = await mount({ autoscaleY: true, manualPriceScale: true });
    await update({ autoscaleY: true });
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
  });

  it('does not reset a chart that mounts with the price axis hidden', async () => {
    await mount({ autoscaleY: true, manualPriceScale: true, theme: { showYAxis: false } });
    expect(resetPriceScale).not.toHaveBeenCalled();
  });
});

describe('price axis without autoscaleY', () => {
  it('scales on a drag as before, whatever manualPriceScale says', async () => {
    const { drag, pinchWheel } = await mount({});
    drag(AXIS_X);
    expect(scalePriceAxis).toHaveBeenCalledWith(40);
    pinchWheel();
    expect(zoom.mock.calls[0]![1]).not.toBe(1);
  });

  it('returns a manual scale to auto-fit on a double-click on the price axis', async () => {
    const { doubleClick } = await mount({});
    doubleClick(PLOT_X);
    expect(resetPriceScale).not.toHaveBeenCalled();
    doubleClick(AXIS_X);
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
  });

  it('keeps the scale when the price axis is hidden', async () => {
    const { update } = await mount({ manualPriceScale: true });
    await update({ manualPriceScale: true, theme: { showYAxis: false } });
    await update({ manualPriceScale: true });
    await update({});
    expect(resetPriceScale).not.toHaveBeenCalled();
  });
});
