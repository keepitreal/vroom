// @vitest-environment jsdom
import { act, cleanup, fireEvent, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle } from '@vroomchart/types';

const scalePriceAxis = vi.fn();
const resetPriceScale = vi.fn();
const translate = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ startMs: 0, endMs: 600_000 }),
    scalePriceAxis: scalePriceAxis as never,
    resetPriceScale: resetPriceScale as never,
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

// The fake strip is the right-hand 60px of an 800px chart.
const AXIS_X = 780;
const PLOT_X = 300;

type Props = { autoscaleY?: boolean };

async function mount(props: Props) {
  const r = render(<VroomChart candles={candles} width={800} height={400} {...props} />);
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
  return { drag, doubleClick };
}

// What the core does with the drag differs (it scales the auto-fit under
// autoscaleY and takes the range manual without it); the gestures that reach it
// are the same either way.
describe.each([
  ['with autoscaleY', { autoscaleY: true }],
  ['without autoscaleY', {}],
] satisfies [string, Props][])('price axis %s', (_name, props) => {
  it('scales price on a drag down the axis', async () => {
    const { drag } = await mount(props);
    drag(AXIS_X);
    expect(scalePriceAxis).toHaveBeenCalledWith(40);
  });

  it('leaves the price scale alone on a drag across the plot', async () => {
    const { drag } = await mount(props);
    drag(PLOT_X);
    expect(translate).toHaveBeenCalled();
    expect(scalePriceAxis).not.toHaveBeenCalled();
  });

  it('resets the price scale on a double-click on the axis only', async () => {
    const { doubleClick } = await mount(props);
    doubleClick(PLOT_X);
    expect(resetPriceScale).not.toHaveBeenCalled();
    doubleClick(AXIS_X);
    expect(resetPriceScale).toHaveBeenCalledTimes(1);
  });
});
