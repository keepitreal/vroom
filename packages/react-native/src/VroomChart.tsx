// VroomChart — Phase 3.
//
// Owns a SharedValue<SkPicture> driven by:
//   - useChartCore's "initial" picture (when data/size/range change), AND
//   - Pan gesture callbacks that call handle.pan(dx, dy) → fresh picture.
//
// Reanimated 4 + RN-Skia 2 propagate SharedValue<SkPicture> changes to
// <Picture> without a React re-render, so gesture-driven redraws are cheap.
//
// Gestures run on the JS thread for now (`runOnJS(true)`) — installing the
// JSI bindings on the worklet runtime is a later perf optimization.

import React, { useEffect, useRef, useCallback } from 'react';
import { View } from 'react-native';
import { Canvas, Picture, type SkPicture } from '@shopify/react-native-skia';
import {
  Gesture,
  GestureDetector,
  GestureHandlerRootView,
} from 'react-native-gesture-handler';
import { useSharedValue, type SharedValue } from 'react-native-reanimated';

import { useChartCore } from './useChartCore';
import type { VroomChartProps } from './types';
import './jsi.d';

export function VroomChart(props: VroomChartProps) {
  const {
    candles,
    width = 360,
    height = 240,
    visibleRange,
    crosshairOffset = 40,
    onViewportChange,
  } = props;

  const { handle, picture } = useChartCore(candles, { width, height }, visibleRange);
  const pictureSV = useSharedValue(picture);

  // When the crosshair is showing, pan moves it (instead of scrolling) and
  // pinch is disabled. A ref (not state) so gesture callbacks read it
  // synchronously without re-subscribing. Tap dismisses it.
  const crosshairActive = useRef(false);

  // Sync the initial picture from useChartCore into the SV whenever it
  // refreshes (data load, size change, externally-controlled range change).
  useEffect(() => {
    pictureSV.value = picture;
  }, [picture, pictureSV]);

  // Momentum scroll. After Pan ends with non-trivial velocity, we run a RAF
  // loop that calls handle.pan(dx, 0) each frame with an exponentially
  // decaying velocity. A new pan (or unmount) cancels the loop.
  const decayRaf = useRef<number | null>(null);
  const cancelDecay = useCallback(() => {
    if (decayRaf.current != null) {
      cancelAnimationFrame(decayRaf.current);
      decayRaf.current = null;
    }
  }, []);
  useEffect(() => cancelDecay, [cancelDecay]);

  // Axis-label fade animation loop. When a gesture changes which labels are
  // active, the C++ side starts ramping their opacities. We keep calling
  // render() on every frame until handle.isAnimating() returns false. The
  // loop is started by gesture callbacks (and the momentum tick) after they
  // update the picture, and self-stops when fades settle.
  const animRaf = useRef<number | null>(null);
  const animTick = useCallback(() => {
    animRaf.current = null;
    if (!handle) return;
    const next = handle.render();
    if (next) pictureSV.value = next;
    if (handle.isAnimating()) {
      animRaf.current = requestAnimationFrame(animTick);
    }
  }, [handle, pictureSV]);
  const maybeStartAnim = useCallback(() => {
    if (animRaf.current != null) return;
    if (!handle?.isAnimating()) return;
    animRaf.current = requestAnimationFrame(animTick);
  }, [handle, animTick]);
  useEffect(() => {
    return () => {
      if (animRaf.current != null) {
        cancelAnimationFrame(animRaf.current);
        animRaf.current = null;
      }
    };
  }, []);

  // Classifies a touch point into the candle area vs. an axis strip. Axis
  // strips always own their gesture (scale price/time) and take priority over
  // the crosshair: an axis touch never opens, moves, or dismisses it.
  const hitAxis = useCallback(
    (x: number, y: number): 'chart' | 'price-axis' | 'time-axis' => {
      if (!handle) return 'chart';
      const { yAxisWidth, xAxisHeight } = handle.getAxisMetrics();
      if (x > width - yAxisWidth) return 'price-axis';
      if (y > height - xAxisHeight) return 'time-axis';
      return 'chart';
    },
    [handle, width, height],
  );

  // Pan can route to three different C++ mutators depending on where it
  // started: the candle area (chart scroll / crosshair move), the y-axis strip
  // (price scale), or the x-axis strip (time scale). We classify on onStart.
  const panMode = useRef<'chart' | 'price-axis' | 'time-axis'>('chart');

  const pan = Gesture.Pan()
    .runOnJS(true)
    .maxPointers(1)  // don't fight Pinch's two-finger gesture
    .onStart((e) => {
      cancelDecay();
      // Always classify — an axis drag controls the axis even while the
      // crosshair is up. Only a chart-area drag interacts with the crosshair.
      panMode.current = hitAxis(e.x, e.y);
    })
    .onChange((e) => {
      if (!handle) return;
      let next: ReturnType<typeof handle.pan> = null;
      if (panMode.current === 'price-axis') {
        next = handle.scalePriceAxis(e.changeY);
      } else if (panMode.current === 'time-axis') {
        next = handle.scaleTimeAxis(e.changeX);
      } else if (crosshairActive.current) {
        // Chart area + crosshair up → the drag moves the crosshair instead of
        // scrolling. Vertical line tracks the finger x; the dot/horizontal line
        // stay lifted `crosshairOffset` px above the fingertip.
        pictureSV.value = handle.setCrosshair(e.x, e.y - crosshairOffset);
        return;
      } else {
        // Chart area: 1-finger drag translates both axes. Horizontal
        // component scrolls time, vertical component slides price bounds
        // (axes follow). Diagonal works naturally.
        next = handle.translate(e.changeX, e.changeY);
      }
      if (next) pictureSV.value = next;
      maybeStartAnim();
    })
    .onEnd((e) => {
      if (!handle) return;
      // A chart-area drag with the crosshair up just moved the crosshair —
      // nothing about the viewport changed, and no momentum.
      if (panMode.current === 'chart' && crosshairActive.current) return;
      onViewportChange?.(0, 0);

      // Axis drags don't get momentum — they're a precise size adjustment.
      if (panMode.current !== 'chart') return;

      let velocity = e.velocityX;  // px/s
      const MIN_LAUNCH = 80;       // ignore tiny flicks
      const MIN_STOP = 8;          // px/s — stop threshold
      const HALF_LIFE_S = 0.35;    // velocity halves every 0.35s
      if (Math.abs(velocity) < MIN_LAUNCH) return;

      let lastTime = performance.now();
      const tick = () => {
        const now = performance.now();
        const dt = (now - lastTime) / 1000;
        lastTime = now;

        // Frame-time-independent exponential decay.
        velocity *= Math.pow(0.5, dt / HALF_LIFE_S);
        const dx = velocity * dt;
        const next = handle.pan(dx, 0);
        if (next) pictureSV.value = next;
        maybeStartAnim();

        if (Math.abs(velocity) > MIN_STOP) {
          decayRaf.current = requestAnimationFrame(tick);
        } else {
          decayRaf.current = null;
        }
      };
      decayRaf.current = requestAnimationFrame(tick);
    });

  // RNGH Pinch's onChange gives cumulative `scale` since gesture start; we
  // turn it into a per-frame multiplicative ratio by dividing by the
  // previous frame's scale.
  const prevScale = useRef(1);
  const pinch = Gesture.Pinch()
    .runOnJS(true)
    .onStart(() => {
      prevScale.current = 1;
    })
    .onChange((e) => {
      if (!handle) return;
      // No zoom while the crosshair is up — it must be dismissed first.
      if (crosshairActive.current) return;
      const ratio = e.scale / prevScale.current;
      prevScale.current = e.scale;
      const next = handle.zoom(ratio, e.focalX, e.focalY);
      if (next) pictureSV.value = next;
      maybeStartAnim();
    });

  // Long press shows the crosshair at the press point. A stationary hold never
  // activates `pan` (it needs movement first), so the chart won't scroll under
  // the hold. The dot/horizontal line are lifted above the fingertip.
  const longPress = Gesture.LongPress()
    .runOnJS(true)
    .onStart((e) => {
      if (!handle) return;
      // A long press on an axis strip controls the axis, never the crosshair.
      if (hitAxis(e.x, e.y) !== 'chart') return;
      cancelDecay();
      crosshairActive.current = true;
      pictureSV.value = handle.setCrosshair(e.x, e.y - crosshairOffset);
    });

  // A tap dismisses the crosshair while it's up; otherwise it's a no-op (so it
  // never interferes with normal pan/pinch).
  const tap = Gesture.Tap()
    .runOnJS(true)
    .onStart((e) => {
      if (!handle || !crosshairActive.current) return;
      // A tap on an axis strip controls the axis, never dismisses the crosshair.
      if (hitAxis(e.x, e.y) !== 'chart') return;
      crosshairActive.current = false;
      pictureSV.value = handle.clearCrosshair();
    });

  const gesture = Gesture.Simultaneous(pan, pinch, longPress, tap);

  return (
    <GestureHandlerRootView style={{ width, height }}>
      <GestureDetector gesture={gesture}>
        <View style={{ width, height }}>
          <Canvas style={{ flex: 1 }}>
            {picture ? (
              // Once the initial picture has landed, pictureSV is guaranteed
              // non-null — gesture callbacks only ever assign non-null values.
              <Picture picture={pictureSV as SharedValue<SkPicture>} />
            ) : null}
          </Canvas>
        </View>
      </GestureDetector>
    </GestureHandlerRootView>
  );
}
