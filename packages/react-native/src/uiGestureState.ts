import type { ChartHandle } from './jsi.d';

export type PanRegion = 'chart' | 'price-axis' | 'time-axis' | 'indicator';

export function panRegion(
  metrics: ReturnType<ChartHandle['getAxisMetrics']>,
  width: number, height: number, x: number, y: number, autoscaleY: boolean,
): PanRegion {
  'worklet';
  if (x > width - metrics.yAxisWidth) return autoscaleY ? 'chart' : 'price-axis';
  if (y > height - metrics.xAxisHeight) return 'time-axis';
  if (metrics.indicatorHeight > 0 && y > height - metrics.xAxisHeight - metrics.indicatorHeight) {
    return 'indicator';
  }
  return 'chart';
}

type Point = { x: number; y: number };
const MIN_SPAN = 24;
const AXIS_RATIO = 0.5;
export function beginPinch(a: Point, b: Point) {
  'worklet';
  const spanX = Math.abs(a.x - b.x), spanY = Math.abs(a.y - b.y);
  return {
    spanX, spanY, dist: Math.max(Math.hypot(spanX, spanY), MIN_SPAN),
    ratioX: 1, ratioY: 1,
    enableX: spanX >= MIN_SPAN && spanX >= spanY * AXIS_RATIO,
    enableY: spanY >= MIN_SPAN && spanY >= spanX * AXIS_RATIO,
  };
}

export function updatePinch(start: ReturnType<typeof beginPinch>, a: Point, b: Point, autoscaleY: boolean) {
  'worklet';
  const next = { ...start };
  if (autoscaleY) {
    next.ratioX = Math.max(Math.hypot(a.x - b.x, a.y - b.y), MIN_SPAN) / start.dist;
  } else {
    if (start.enableX) next.ratioX = Math.max(Math.abs(a.x - b.x), MIN_SPAN) / start.spanX;
    if (start.enableY) next.ratioY = Math.max(Math.abs(a.y - b.y), MIN_SPAN) / start.spanY;
  }
  return {
    next, scaleX: next.ratioX / start.ratioX, scaleY: next.ratioY / start.ratioY,
    x: (a.x + b.x) * 0.5, y: (a.y + b.y) * 0.5,
  };
}

/** Same decay as the JS path, with cancellation and scheduling owned by UI. */
export function startUiPanDecay(
  handle: Pick<ChartHandle, 'pan'>, epoch: { value: number },
  draw: () => void, velocity: number,
) {
  'worklet';
  if (Math.abs(velocity) < 80) return;
  const token = ++epoch.value;
  let last = performance.now();
  const tick = () => {
    if (epoch.value !== token) return;
    const now = performance.now(), dt = (now - last) / 1000;
    last = now;
    velocity *= Math.pow(0.5, dt / 0.35);
    handle.pan(velocity * dt, 0);
    draw();
    if (Math.abs(velocity) > 8) requestAnimationFrame(tick);
  };
  requestAnimationFrame(tick);
}
