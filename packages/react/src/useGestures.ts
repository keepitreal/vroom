// Pointer/wheel gesture controller for the web chart. Maps input to the same
// VroomChartHandle mutators the React Native gestures use (see
// packages/react-native/src/VroomChart.tsx), so behavior matches across
// platforms. Listeners are attached natively (not via React props) so wheel can
// be non-passive and call preventDefault.

import { useEffect, useRef } from 'react';
import type { CrosshairEvent } from '@vroom/types';
import type { VroomChartHandle } from '@vroom/core-wasm';

type Region = 'chart' | 'price-axis' | 'time-axis' | 'indicator';

export type GestureOptions = {
  crosshairOffset: number;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};

const MIN_SPAN = 24; // px — minimum two-finger span for an axis to scale
const AXIS_RATIO = 0.5; // an axis scales only if its span ≥ this × the other's
const LONG_PRESS_MS = 350;
const MOVE_THRESH = 6; // px before a press becomes a drag
const WHEEL_K = 0.0015; // wheel delta → zoom factor exponent

export function useGestures(
  containerRef: React.RefObject<HTMLElement | null>,
  handleRef: React.RefObject<VroomChartHandle | null>,
  scheduleRender: () => void,
  opts: GestureOptions,
): void {
  // Keep latest opts in a ref so the effect's listeners stay stable.
  const optsRef = useRef(opts);
  optsRef.current = opts;

  useEffect(() => {
    const el = containerRef.current;
    if (!el) return;

    const pointers = new Map<number, { x: number; y: number }>();
    let panMode: Region = 'chart';
    let crosshairActive = false;
    let crosshairSource: 'press' | 'hover' | null = null;
    let lastCrosshairTime: number | null = null;
    let longPressTimer: ReturnType<typeof setTimeout> | null = null;
    let downX = 0;
    let downY = 0;
    let moved = false;
    const pinch = { spanX: 1, spanY: 1, ratioX: 1, ratioY: 1, enableX: false, enableY: false, active: false };

    const rel = (e: PointerEvent | WheelEvent) => {
      const r = el.getBoundingClientRect();
      return { x: e.clientX - r.left, y: e.clientY - r.top };
    };

    const regionAt = (x: number, y: number): Region => {
      const h = handleRef.current;
      const r = el.getBoundingClientRect();
      if (!h) return 'chart';
      const { yAxisWidth, xAxisHeight, indicatorHeight } = h.getAxisMetrics();
      if (x > r.width - yAxisWidth) return 'price-axis';
      if (y > r.height - xAxisHeight) return 'time-axis';
      if (indicatorHeight > 0 && y > r.height - xAxisHeight - indicatorHeight) return 'indicator';
      return 'chart';
    };

    const reportCrosshair = (reason: CrosshairEvent['reason']) => {
      const h = handleRef.current;
      if (!h) return;
      const c = h.getCrosshairCandle();
      const t = c?.timeMs ?? null;
      if (reason === 'move' && t === lastCrosshairTime) return;
      lastCrosshairTime = t;
      optsRef.current.onCrosshair?.({ active: reason !== 'hide', candle: c, reason });
    };

    const clearLongPress = () => {
      if (longPressTimer != null) {
        clearTimeout(longPressTimer);
        longPressTimer = null;
      }
    };

    const showCrosshair = (x: number, y: number, source: 'press' | 'hover', reason: 'show' | 'move') => {
      const h = handleRef.current;
      if (!h) return;
      crosshairActive = true;
      crosshairSource = source;
      const lift = source === 'press' ? optsRef.current.crosshairOffset : 0;
      h.setCrosshair(x, y - lift);
      scheduleRender();
      reportCrosshair(reason);
    };

    const hideCrosshair = () => {
      const h = handleRef.current;
      if (!h || !crosshairActive) return;
      crosshairActive = false;
      crosshairSource = null;
      h.clearCrosshair();
      scheduleRender();
      reportCrosshair('hide');
    };

    const onPointerDown = (e: PointerEvent) => {
      const h = handleRef.current;
      if (!h) return;
      if (e.pointerType === 'mouse' && e.button !== 0) return; // left button only
      const { x, y } = rel(e);
      el.setPointerCapture(e.pointerId);
      pointers.set(e.pointerId, { x, y });

      if (pointers.size === 2) {
        clearLongPress();
        hideCrosshair();
        const pts = [...pointers.values()];
        const sx = Math.abs(pts[0]!.x - pts[1]!.x);
        const sy = Math.abs(pts[0]!.y - pts[1]!.y);
        pinch.spanX = sx;
        pinch.spanY = sy;
        pinch.ratioX = 1;
        pinch.ratioY = 1;
        pinch.enableX = sx >= MIN_SPAN && sx >= sy * AXIS_RATIO;
        pinch.enableY = sy >= MIN_SPAN && sy >= sx * AXIS_RATIO;
        pinch.active = true;
        return;
      }

      // Single pointer: classify region; arm long-press → crosshair (touch/pen).
      downX = x;
      downY = y;
      moved = false;
      panMode = regionAt(x, y);
      if (e.pointerType !== 'mouse' && panMode === 'chart') {
        longPressTimer = setTimeout(() => {
          longPressTimer = null;
          if (!moved) showCrosshair(downX, downY, 'press', 'show');
        }, LONG_PRESS_MS);
      }
    };

    const onPointerMove = (e: PointerEvent) => {
      const h = handleRef.current;
      if (!h) return;
      const { x, y } = rel(e);

      // Hover (mouse, no button) → crosshair follows the cursor on the chart.
      if (!pointers.has(e.pointerId)) {
        if (e.pointerType === 'mouse' && pointers.size === 0) {
          if (regionAt(x, y) === 'chart') {
            showCrosshair(x, y, 'hover', crosshairActive ? 'move' : 'show');
          } else {
            hideCrosshair();
          }
        }
        return;
      }

      const prev = pointers.get(e.pointerId)!;
      const dx = x - prev.x;
      const dy = y - prev.y;
      pointers.set(e.pointerId, { x, y });

      // Two-finger directional pinch.
      if (pointers.size >= 2 && pinch.active) {
        const pts = [...pointers.values()];
        const focalX = (pts[0]!.x + pts[1]!.x) / 2;
        const focalY = (pts[0]!.y + pts[1]!.y) / 2;
        let frameX = 1;
        if (pinch.enableX) {
          const r = Math.max(Math.abs(pts[0]!.x - pts[1]!.x), MIN_SPAN) / pinch.spanX;
          frameX = r / pinch.ratioX;
          pinch.ratioX = r;
        }
        let frameY = 1;
        if (pinch.enableY) {
          const r = Math.max(Math.abs(pts[0]!.y - pts[1]!.y), MIN_SPAN) / pinch.spanY;
          frameY = r / pinch.ratioY;
          pinch.ratioY = r;
        }
        if (frameX !== 1 || frameY !== 1) {
          h.zoom(frameX, frameY, focalX, focalY);
          scheduleRender();
        }
        return;
      }

      if (!moved && Math.hypot(x - downX, y - downY) > MOVE_THRESH) {
        moved = true;
        clearLongPress();
      }
      if (!moved) return;

      // Chart-area drag with the (press) crosshair up moves the crosshair.
      if (crosshairActive && crosshairSource === 'press' && panMode === 'chart') {
        showCrosshair(x, y, 'press', 'move');
        return;
      }

      if (panMode === 'price-axis') h.scalePriceAxis(dy);
      else if (panMode === 'time-axis') h.scaleTimeAxis(dx);
      else if (panMode === 'indicator') h.pan(dx, 0);
      else h.translate(dx, dy);
      scheduleRender();
    };

    const endPointer = (e: PointerEvent) => {
      const had = pointers.delete(e.pointerId);
      clearLongPress();
      if (pointers.size < 2) pinch.active = false;
      if (!had) return;

      // Press crosshair is active only while held — release dismisses it.
      if (crosshairSource === 'press' && pointers.size === 0) {
        hideCrosshair();
      } else if (moved && (panMode === 'chart' || panMode === 'indicator')) {
        optsRef.current.onViewportChange?.(0, 0);
      }
    };

    const onPointerLeave = () => {
      if (crosshairSource === 'hover') hideCrosshair();
    };

    const onWheel = (e: WheelEvent) => {
      const h = handleRef.current;
      if (!h) return;
      e.preventDefault();
      const { x, y } = rel(e);
      if (e.ctrlKey || e.metaKey) {
        const f = Math.exp(-e.deltaY * WHEEL_K);
        h.zoom(f, f, x, y);
      } else if (e.shiftKey) {
        h.pan(-e.deltaY, 0);
      } else {
        const f = Math.exp(-e.deltaY * WHEEL_K);
        h.zoom(f, 1, x, y);
      }
      scheduleRender();
    };

    el.addEventListener('pointerdown', onPointerDown);
    el.addEventListener('pointermove', onPointerMove);
    el.addEventListener('pointerup', endPointer);
    el.addEventListener('pointercancel', endPointer);
    el.addEventListener('pointerleave', onPointerLeave);
    el.addEventListener('wheel', onWheel, { passive: false });

    return () => {
      clearLongPress();
      el.removeEventListener('pointerdown', onPointerDown);
      el.removeEventListener('pointermove', onPointerMove);
      el.removeEventListener('pointerup', endPointer);
      el.removeEventListener('pointercancel', endPointer);
      el.removeEventListener('pointerleave', onPointerLeave);
      el.removeEventListener('wheel', onWheel);
    };
  }, [containerRef, handleRef, scheduleRender]);
}
