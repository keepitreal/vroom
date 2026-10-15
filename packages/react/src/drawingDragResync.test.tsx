// @vitest-environment jsdom
import { act, cleanup, fireEvent, render } from '@testing-library/react';
import { afterEach, beforeAll, describe, expect, it, vi } from 'vitest';
import type { Candle, DrawPoint, Drawing } from '@vroomchart/types';
import type { DrawingSpec } from '@vroomchart/core-wasm';

// A stand-in for the WASM handle that keeps just enough drawing state to check
// what the core would paint: setDrawings replaces it, the edit mutators change
// it. Everything else is a no-op.
type Hit = { index: number; part: number; t: number } | null;
const core = {
  drawings: [] as DrawPoint[][],
  hit: null as Hit,
  setDrawingsCalls: 0,
};

function fakeHandle() {
  const impl: Record<string, (...args: never[]) => unknown> = {
    setDrawings: (specs: DrawingSpec[]) => {
      core.setDrawingsCalls++;
      core.drawings = specs.map((s) =>
        s.points
          ? s.points.map((p) => ({ ...p }))
          : [
              { timeMs: s.aTime, price: s.aPrice },
              { timeMs: s.bTime, price: s.bPrice },
            ],
      );
    },
    moveDrawingEndpoint: (i: number, ep: number, timeMs: number, price: number) => {
      const d = core.drawings[i];
      if (d) d[ep === 0 ? 0 : d.length - 1] = { timeMs, price };
    },
    moveDrawingVertex: (i: number, v: number, timeMs: number, price: number) => {
      const d = core.drawings[i];
      if (d?.[v]) d[v] = { timeMs, price };
    },
    translateDrawing: (i: number, dT: number, dP: number) => {
      const d = core.drawings[i];
      if (d) core.drawings[i] = d.map((p) => ({ timeMs: p.timeMs + dT, price: p.price + dP }));
    },
    hitTestDrawing: () => core.hit,
    coordAt: (x: number, y: number) => ({ timeMs: x * 1000, price: y }),
    project: (timeMs: number, price: number) => ({ x: timeMs / 1000, y: price }),
    getAxisMetrics: () => ({ yAxisWidth: 60, xAxisHeight: 24, indicatorHeight: 0 }),
    getVisibleRange: () => ({ startMs: 0, endMs: 1_000_000 }),
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
  core.drawings = [];
  core.hit = null;
  core.setDrawingsCalls = 0;
});

const candlesAt = (n: number): Candle[] =>
  Array.from({ length: n }, (_, i) => ({
    timeMs: i * 60_000,
    open: 100,
    high: 101,
    low: 99,
    close: 100 + i,
    volume: 1,
  }));

const line: Drawing = {
  id: 'line',
  type: 'line',
  points: [
    { timeMs: 100_000, price: 100 },
    { timeMs: 300_000, price: 200 },
  ],
};
const box: Drawing = {
  id: 'box',
  type: 'box',
  points: [
    { timeMs: 100_000, price: 100 },
    { timeMs: 300_000, price: 200 },
  ],
};
const pencil: Drawing = {
  id: 'pencil',
  type: 'pencil',
  points: [
    { timeMs: 100_000, price: 100 },
    { timeMs: 150_000, price: 120 },
    { timeMs: 200_000, price: 110 },
  ],
};

async function mount(drawings: Drawing[], onDrawingChange = vi.fn()) {
  let candles = candlesAt(10);
  const ui = (d: Drawing[]) => (
    <VroomChart
      candles={candles}
      width={800}
      height={400}
      drawings={d}
      onDrawingChange={onDrawingChange}
    />
  );
  const r = render(ui(drawings));
  await act(async () => {});
  const el = r.container.querySelector('canvas')!.parentElement!;
  const pointer = (type: 'down' | 'move' | 'up', x: number, y: number) =>
    act(() => {
      const init = { pointerId: 1, pointerType: 'mouse', button: 0, clientX: x, clientY: y };
      if (type === 'down') fireEvent.pointerDown(el, init);
      else if (type === 'move') fireEvent.pointerMove(el, init);
      else fireEvent.pointerUp(el, init);
    });
  // A streamed candle: a new array, same drawings prop.
  const tick = (d: Drawing[] = drawings) => {
    candles = candlesAt(candles.length + 1);
    act(() => r.rerender(ui(d)));
  };
  // Click a body to select it (a translate needs the shape selected first).
  const select = (index: number, part: number, x: number, y: number) => {
    core.hit = { index, part, t: 0.5 };
    pointer('down', x, y);
    pointer('up', x, y);
  };
  return { pointer, tick, select, onDrawingChange };
}

describe('live drawing edits survive a props re-sync', () => {
  it('keeps a dragged line endpoint under the pointer', async () => {
    const { pointer, tick, onDrawingChange } = await mount([line]);
    core.hit = { index: 0, part: 1, t: 1 };
    pointer('down', 300, 200);
    pointer('move', 350, 250);
    tick();
    expect(core.drawings[0]).toEqual([
      { timeMs: 100_000, price: 100 },
      { timeMs: 350_000, price: 250 },
    ]);
    pointer('up', 350, 250);
    expect(onDrawingChange).toHaveBeenCalledTimes(1);
    expect(onDrawingChange.mock.calls[0]![0].points[1]).toEqual({ timeMs: 350_000, price: 250 });
  });

  it('keeps a box corner drag and its fixed diagonal', async () => {
    const { pointer, tick } = await mount([box]);
    // Corner 2 is (b.time, b.price); its diagonal is corner 0, (a.time, a.price).
    core.hit = { index: 0, part: 2, t: 0 };
    pointer('down', 300, 200);
    pointer('move', 320, 230);
    tick();
    expect(core.drawings[0]).toEqual([
      { timeMs: 320_000, price: 230 },
      { timeMs: 100_000, price: 100 },
    ]);
  });

  it('keeps a translated line at the dragged position', async () => {
    const { pointer, tick, select } = await mount([line]);
    select(0, 2, 200, 150);
    pointer('down', 200, 150);
    pointer('move', 230, 170);
    tick();
    expect(core.drawings[0]).toEqual([
      { timeMs: 130_000, price: 120 },
      { timeMs: 330_000, price: 220 },
    ]);
  });

  it('keeps a translated pencil exact across several re-syncs', async () => {
    const { pointer, tick, select, onDrawingChange } = await mount([pencil]);
    select(0, 5, 150, 120);
    pointer('down', 150, 120);
    pointer('move', 160, 125);
    tick();
    pointer('move', 175, 140);
    tick();
    tick();
    pointer('move', 190, 150);
    tick();
    const expected = pencil.points.map((p) => ({ timeMs: p.timeMs + 40_000, price: p.price + 30 }));
    expect(core.drawings[0]).toEqual(expected);
    pointer('up', 190, 150);
    expect(onDrawingChange).toHaveBeenCalledTimes(1);
    expect(onDrawingChange.mock.calls[0]![0].points).toEqual(expected);
  });

  it('cancels the drag when the host replaces the drawing', async () => {
    const { pointer, tick, onDrawingChange } = await mount([line]);
    core.hit = { index: 0, part: 1, t: 1 };
    pointer('down', 300, 200);
    pointer('move', 350, 250);
    // A different drawing now sits at the dragged index.
    const other: Drawing = { ...line, id: 'other' };
    tick([other]);
    pointer('move', 360, 260);
    pointer('up', 360, 260);
    expect(core.drawings[0]).toEqual(other.points);
    expect(onDrawingChange).not.toHaveBeenCalled();
  });

  it('still applies updated drawings when nothing is being dragged', async () => {
    const { tick } = await mount([line]);
    const moved: Drawing = {
      ...line,
      points: [
        { timeMs: 500_000, price: 50 },
        { timeMs: 600_000, price: 60 },
      ],
    };
    tick([moved]);
    expect(core.drawings[0]).toEqual(moved.points);
  });
});
