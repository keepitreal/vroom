import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { beginPinch, panRegion, startUiPanDecay, updatePinch } from './uiGestureState';

const metrics = { yAxisWidth: 40, xAxisHeight: 20, indicatorHeight: 100 };

describe('UI gesture routing', () => {
  it('gives the price strip priority at the corner, unless autoscaling owns it', () => {
    expect(panRegion(metrics, 400, 600, 380, 590, false)).toBe('price-axis');
    expect(panRegion(metrics, 400, 600, 380, 590, true)).toBe('chart');
  });
  it('distinguishes plot, indicator and time strip', () => {
    expect(panRegion(metrics, 400, 600, 200, 400, false)).toBe('chart');
    expect(panRegion(metrics, 400, 600, 200, 530, false)).toBe('indicator');
    expect(panRegion(metrics, 400, 600, 200, 590, false)).toBe('time-axis');
  });
});

describe('UI directional pinch', () => {
  it('keeps a horizontal pinch off the price scale', () => {
    const start = beginPinch({ x: 40, y: 100 }, { x: 160, y: 100 });
    const step = updatePinch(start, { x: 30, y: 95 }, { x: 170, y: 105 }, false);
    expect(step.scaleX).toBeCloseTo(140 / 120);
    expect(step.scaleY).toBe(1);
  });
  it('keeps a vertical pinch off time, including crossing x coordinates', () => {
    const start = beginPinch({ x: 100, y: 40 }, { x: 100, y: 160 });
    const step = updatePinch(start, { x: 105, y: 20 }, { x: 95, y: 180 }, false);
    expect(step.scaleX).toBe(1);
    expect(step.scaleY).toBeCloseTo(160 / 120);
  });
  it('uses incremental ratios and the current focal point', () => {
    const start = beginPinch({ x: 50, y: 50 }, { x: 150, y: 150 });
    const first = updatePinch(start, { x: 0, y: 0 }, { x: 200, y: 200 }, false);
    const second = updatePinch(first.next, { x: -25, y: -25 }, { x: 225, y: 225 }, false);
    expect(first.scaleX).toBe(2);
    expect(second.scaleX).toBe(1.25);
    expect(second.scaleY).toBe(1.25);
    expect([second.x, second.y]).toEqual([100, 100]);
  });
  it('maps any orientation to time-only zoom under autoscaleY', () => {
    const start = beginPinch({ x: 100, y: 40 }, { x: 100, y: 160 });
    const step = updatePinch(start, { x: 100, y: -20 }, { x: 100, y: 220 }, true);
    expect(step.scaleX).toBe(2);
    expect(step.scaleY).toBe(1);
  });
  it('floors spans when fingers cross and ignores tiny initial axes', () => {
    const start = beginPinch({ x: 40, y: 100 }, { x: 160, y: 100 });
    const step = updatePinch(start, { x: 100, y: 100 }, { x: 100, y: 100 }, false);
    expect(step.scaleX).toBe(24 / 120);
    const tiny = beginPinch({ x: 0, y: 0 }, { x: 1, y: 1 });
    const ignored = updatePinch(tiny, { x: 0, y: 0 }, { x: 100, y: 100 }, false);
    expect([ignored.scaleX, ignored.scaleY]).toEqual([1, 1]);
  });
});

describe('UI momentum', () => {
  let callbacks: (() => void)[];
  let now: number;
  const frame = () => {
    now += 1000 / 60;
    const pending = callbacks;
    callbacks = [];
    pending.forEach((f) => f());
  };
  beforeEach(() => {
    callbacks = []; now = 0;
    vi.spyOn(performance, 'now').mockImplementation(() => now);
    vi.stubGlobal('requestAnimationFrame', (f: () => void) => callbacks.push(f));
  });
  afterEach(() => { vi.restoreAllMocks(); vi.unstubAllGlobals(); });
  it('does not launch a tiny flick', () => {
    startUiPanDecay({ pan: vi.fn() }, { value: 0 }, vi.fn(), 79);
    expect(callbacks).toHaveLength(0);
  });
  it('presents native pan frames and decays to a stop', () => {
    const pan = vi.fn(), draw = vi.fn();
    startUiPanDecay({ pan }, { value: 0 }, draw, -160);
    let frames = 0;
    while (callbacks.length && frames++ < 200) frame();
    expect(callbacks).toHaveLength(0);
    expect(pan.mock.calls[0][0]).toBeLessThan(0);
    expect(Math.abs(pan.mock.calls.at(-1)![0])).toBeLessThan(Math.abs(pan.mock.calls[0][0]));
    expect(draw).toHaveBeenCalledTimes(pan.mock.calls.length);
  });
  it('cancels a queued frame on a new interaction or unmount', () => {
    const pan = vi.fn(), draw = vi.fn(), epoch = { value: 0 };
    startUiPanDecay({ pan }, epoch, draw, 200);
    epoch.value++;
    frame();
    expect(pan).not.toHaveBeenCalled();
    expect(draw).not.toHaveBeenCalled();
    expect(callbacks).toHaveLength(0);
  });
  it('a replacement fling does not keep the previous loop alive', () => {
    const pan = vi.fn(), epoch = { value: 0 };
    startUiPanDecay({ pan }, epoch, vi.fn(), -200);
    startUiPanDecay({ pan }, epoch, vi.fn(), 200);
    frame();
    expect(pan).toHaveBeenCalledTimes(1);
    expect(pan.mock.calls[0][0]).toBeGreaterThan(0);
  });
});
