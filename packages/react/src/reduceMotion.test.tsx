// @vitest-environment jsdom
import { act, cleanup, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import { FloatKey } from '@vroomchart/core-wasm';
import type { Candle } from '@vroomchart/types';

const setFloat = vi.fn();
const setAutoscaleY = vi.fn();

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ startMs: 0, endMs: 600_000 }),
    setFloat: setFloat as never,
    setAutoscaleY: setAutoscaleY as never,
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
  setFloat.mockClear();
  setAutoscaleY.mockClear();
  vi.unstubAllGlobals();
});

const candles: Candle[] = Array.from({ length: 10 }, (_, i) => ({
  timeMs: i * 60_000,
  open: 100,
  high: 101,
  low: 99,
  close: 100,
  volume: 1,
}));

function stubOsReducedMotion(matches: boolean) {
  vi.stubGlobal('matchMedia', () => ({ matches }));
  window.matchMedia = (() => ({ matches })) as unknown as typeof window.matchMedia;
}

async function mount(reduceMotion?: boolean) {
  const r = render(
    <VroomChart
      candles={candles}
      width={800}
      height={400}
      autoscaleY
      reduceMotion={reduceMotion}
    />,
  );
  await act(async () => {});
  return r;
}

const lastReduceMotion = () =>
  setFloat.mock.calls.filter(([key]) => key === FloatKey.ReduceMotion).at(-1)?.[1];

describe('reduceMotion', () => {
  it('leaves the core animating by default', async () => {
    stubOsReducedMotion(false);
    await mount();
    expect(lastReduceMotion()).toBe(0);
    expect(setAutoscaleY).toHaveBeenLastCalledWith(true, true);
  });

  it('snaps the core and the autoscale refit when the prop is true', async () => {
    stubOsReducedMotion(false);
    await mount(true);
    expect(lastReduceMotion()).toBe(1);
    expect(setAutoscaleY).toHaveBeenLastCalledWith(true, false);
  });

  it('turns on when the prop flips after mount', async () => {
    stubOsReducedMotion(false);
    const r = await mount(false);
    expect(lastReduceMotion()).toBe(0);
    r.rerender(
      <VroomChart candles={candles} width={800} height={400} autoscaleY reduceMotion />,
    );
    await act(async () => {});
    expect(lastReduceMotion()).toBe(1);
    expect(setAutoscaleY).toHaveBeenLastCalledWith(true, false);
  });

  it('cannot override an OS reduced-motion request', async () => {
    stubOsReducedMotion(true);
    await mount(false);
    expect(lastReduceMotion()).toBe(1);
    expect(setAutoscaleY).toHaveBeenLastCalledWith(true, false);
  });

  it('restores the tip pulse when reduced motion is turned back off', async () => {
    stubOsReducedMotion(false);
    const lastPulse = () =>
      setFloat.mock.calls.filter(([key]) => key === FloatKey.LineTipPulse).at(-1)?.[1];
    const chart = (reduce: boolean) => (
      <VroomChart
        candles={candles}
        width={800}
        height={400}
        theme={{ lineTipPulse: true }}
        reduceMotion={reduce}
      />
    );
    const r = render(chart(true));
    await act(async () => {});
    expect(lastPulse()).toBe(0);
    r.rerender(chart(false));
    await act(async () => {});
    expect(lastPulse()).toBe(1);
  });
});
