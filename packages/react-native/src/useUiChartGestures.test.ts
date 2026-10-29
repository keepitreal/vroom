import { beforeEach, describe, expect, it, vi } from 'vitest';
import { usePanGesture, useTapGesture } from 'react-native-gesture-handler';
import type { ChartHandle } from './jsi.d';
import { useUiChartGestures } from './useUiChartGestures';

vi.mock('react-native-gesture-handler', () => ({
  usePanGesture: vi.fn(), usePinchGesture: vi.fn(),
  useLongPressGesture: vi.fn(), useTapGesture: vi.fn(),
}));
vi.mock('react-native-reanimated', () => ({
  useSharedValue: (value: unknown) => ({ value }),
  runOnJS: (callback: unknown) => callback,
}));

function gestures() {
  const crosshairActive = { value: true };
  const info = { timeMs: 120, price: 10, indicatorKind: -1, indicatorValue: null, candle: null };
  const handle = {
    getAxisMetrics: () => ({ yAxisWidth: 40, xAxisHeight: 20, indicatorHeight: 0 }),
    setCrosshair: vi.fn(), getCrosshairInfo: () => info, clearCrosshair: vi.fn(),
    translate: vi.fn(), pan: vi.fn(),
  };
  const draw = vi.fn(), notifyCrosshair = vi.fn();
  useUiChartGestures({
    enabled: true, handle: handle as unknown as ChartHandle, draw,
    width: 400, height: 600, autoscaleY: false, crosshairEnabled: true,
    crosshairOffset: 30, crosshairActive, epoch: { value: 0 },
    notifyCrosshair, notifyViewport: vi.fn(),
  });
  return {
    handle, draw, notifyCrosshair, crosshairActive, info,
    pan: vi.mocked(usePanGesture).mock.calls.at(-1)![0]!,
    tap: vi.mocked(useTapGesture).mock.calls.at(-1)![0]!,
  };
}

beforeEach(() => vi.clearAllMocks());

describe('UI crosshair gestures', () => {
  it('limits tap travel so an Android drag cannot also dismiss the crosshair', () => {
    expect(gestures().tap.maxDistance).toBe(10);
  });
  it('moves an active crosshair without clearing it or translating the viewport', () => {
    const g = gestures();
    const event = { x: 180, y: 220, changeX: 25, changeY: 0 };
    g.pan.onActivate?.(event as Parameters<NonNullable<typeof g.pan.onActivate>>[0]);
    const update = g.pan.onUpdate;
    if (typeof update !== 'function') throw new Error('Expected a pan callback');
    update(event as Parameters<typeof update>[0]);
    expect(g.crosshairActive.value).toBe(true);
    expect(g.handle.setCrosshair).toHaveBeenCalledWith(180, 190);
    expect(g.handle.clearCrosshair).not.toHaveBeenCalled();
    expect(g.handle.translate).not.toHaveBeenCalled();
    expect(g.handle.pan).not.toHaveBeenCalled();
    expect(g.notifyCrosshair).toHaveBeenCalledWith(g.info, 'move');
    expect(g.draw).toHaveBeenCalled();
  });
  it('still dismisses the crosshair on a recognized chart tap', () => {
    const g = gestures();
    g.tap.onActivate?.({ x: 180, y: 220 } as Parameters<NonNullable<typeof g.tap.onActivate>>[0]);
    expect(g.crosshairActive.value).toBe(false);
    expect(g.handle.clearCrosshair).toHaveBeenCalledOnce();
    expect(g.notifyCrosshair).toHaveBeenCalledWith(null, 'hide');
    expect(g.draw).toHaveBeenCalled();
  });
});
