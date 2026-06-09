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

import React, { useEffect, useRef, useCallback, useState, useMemo } from 'react';
import { View, type LayoutChangeEvent } from 'react-native';
import { Canvas, Picture, Skia, type SkPicture } from '@shopify/react-native-skia';
import {
  Gesture,
  GestureDetector,
  GestureHandlerRootView,
} from 'react-native-gesture-handler';
import { useSharedValue } from 'react-native-reanimated';

import { useChartCore } from './useChartCore';
import type { VroomChartProps } from './types';
import './jsi.d';

export function VroomChart(props: VroomChartProps) {
  const {
    candles,
    width: widthProp,
    height: heightProp,
    style,
    visibleRange,
    theme,
    crosshairOffset = 40,
    onCrosshair,
    onViewportChange,
  } = props;

  // Fill the parent by default: measure via onLayout. Explicit width/height
  // props (if given) win per-axis. Until the first layout, dims are 0 and we
  // render nothing (one frame).
  const [measured, setMeasured] = useState({ width: 0, height: 0 });
  const width = widthProp ?? measured.width;
  const height = heightProp ?? measured.height;

  const onLayout = useCallback((e: LayoutChangeEvent) => {
    const w = Math.round(e.nativeEvent.layout.width);
    const h = Math.round(e.nativeEvent.layout.height);
    setMeasured((prev) =>
      prev.width === w && prev.height === h ? prev : { width: w, height: h },
    );
  }, []);

  const { handle, picture } = useChartCore(
    candles,
    { width, height },
    visibleRange,
    theme,
  );

  // RN-Skia's recorder reads this SharedValue on the UI/render runtime, a beat
  // behind JS-thread writes. If it ever reads null it throws ("Invalid prop
  // value for SkTextBlob received" — RN-Skia's mislabeled SkPicture error), so
  // we seed it with an empty picture and *never* assign null into it.
  const emptyPicture = useMemo(() => {
    const rec = Skia.PictureRecorder();
    rec.beginRecording(Skia.XYWHRect(0, 0, 1, 1));
    return rec.finishRecordingAsPicture();
  }, []);
  const pictureSV = useSharedValue<SkPicture>(emptyPicture);

  // When the crosshair is showing, pan moves it (instead of scrolling) and
  // pinch is disabled. A ref (not state) so gesture callbacks read it
  // synchronously without re-subscribing. Tap dismisses it.
  const crosshairActive = useRef(false);

  // timeMs of the candle last reported through onCrosshair, so a drag fires a
  // 'move' event only when it crosses into a *different* candle (one per
  // candle, not per frame). Null while the crosshair is hidden.
  const lastCrosshairTime = useRef<number | null>(null);

  // Sync the initial picture from useChartCore into the SV whenever it
  // refreshes (data load, size change, externally-controlled range change).
  // Only ever assign a non-null picture (see emptyPicture note above).
  useEffect(() => {
    if (picture) pictureSV.value = picture;
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
        const ch = handle.setCrosshair(e.x, e.y - crosshairOffset);
        if (ch) pictureSV.value = ch;
        // The line follows the finger every frame (above), but only notify the
        // host when the snapped candle actually changes.
        const c = handle.getCrosshairCandle();
        const t = c?.timeMs ?? null;
        if (t !== lastCrosshairTime.current) {
          lastCrosshairTime.current = t;
          onCrosshair?.({ active: true, candle: c, reason: 'move' });
        }
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

  // Directional pinch. A single Pinch scale is uniform, so we read the two
  // touch points and track their horizontal/vertical spans independently: a
  // vertical pinch scales price (y), a horizontal pinch scales the time window
  // (x), and a diagonal pinch does both. An axis whose initial span is tiny
  // (fingers ~collinear on that axis) is left alone.
  // Lock the scalable axes at gesture start by orientation: an axis only
  // scales if its initial span is meaningful AND at least AXIS_RATIO of the
  // other axis. This keeps a vertical pinch from ever touching x (and vice
  // versa) — critical because during a vertical pinch the fingers' x-coords
  // drift and cross, sending spanX through ~0 and otherwise exploding frameX.
  const MIN_SPAN = 24;     // px — minimum span for an axis to scale at all
  const AXIS_RATIO = 0.5;  // axis scales only if its span ≥ this × the other's
  const pinchStart = useRef({
    spanX: 1,
    spanY: 1,
    ratioX: 1,
    ratioY: 1,
    enableX: false,
    enableY: false,
  });
  const pinch = Gesture.Pinch()
    .runOnJS(true)
    .onTouchesDown((e) => {
      if (e.numberOfTouches < 2) return;
      const [a, b] = e.allTouches;
      const spanX = Math.abs(a.x - b.x);
      const spanY = Math.abs(a.y - b.y);
      pinchStart.current = {
        spanX,
        spanY,
        ratioX: 1,
        ratioY: 1,
        enableX: spanX >= MIN_SPAN && spanX >= spanY * AXIS_RATIO,
        enableY: spanY >= MIN_SPAN && spanY >= spanX * AXIS_RATIO,
      };
    })
    .onTouchesMove((e) => {
      if (!handle || crosshairActive.current) return;
      if (e.numberOfTouches < 2) return;
      const [a, b] = e.allTouches;
      const start = pinchStart.current;
      const focalX = (a.x + b.x) * 0.5;
      const focalY = (a.y + b.y) * 0.5;

      // Per-frame factor = current cumulative ratio / previous. Floor the
      // current span at MIN_SPAN so a near-zero span (fingers crossing on that
      // axis) can't blow the ratio up.
      let frameX = 1;
      if (start.enableX) {
        const ratioX = Math.max(Math.abs(a.x - b.x), MIN_SPAN) / start.spanX;
        frameX = ratioX / start.ratioX;
        start.ratioX = ratioX;
      }
      let frameY = 1;
      if (start.enableY) {
        const ratioY = Math.max(Math.abs(a.y - b.y), MIN_SPAN) / start.spanY;
        frameY = ratioY / start.ratioY;
        start.ratioY = ratioY;
      }
      if (frameX === 1 && frameY === 1) return;

      const next = handle.zoom(frameX, frameY, focalX, focalY);
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
      const ch = handle.setCrosshair(e.x, e.y - crosshairOffset);
      if (ch) pictureSV.value = ch;
      const c = handle.getCrosshairCandle();
      lastCrosshairTime.current = c?.timeMs ?? null;
      onCrosshair?.({ active: true, candle: c, reason: 'show' });
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
      const ch = handle.clearCrosshair();
      if (ch) pictureSV.value = ch;
      lastCrosshairTime.current = null;
      onCrosshair?.({ active: false, candle: null, reason: 'hide' });
    });

  const gesture = Gesture.Simultaneous(pan, pinch, longPress, tap);

  return (
    <GestureHandlerRootView
      onLayout={onLayout}
      style={[
        { width: widthProp, height: heightProp },
        widthProp == null && heightProp == null ? { flex: 1 } : null,
        style,
      ]}
    >
      <GestureDetector gesture={gesture}>
        <View style={{ flex: 1 }}>
          <Canvas style={{ flex: 1 }}>
            {width > 0 && height > 0 ? (
              // pictureSV is always a valid picture (seeded empty, never null),
              // so RN-Skia's UI-thread reader never sees null.
              <Picture picture={pictureSV} />
            ) : null}
          </Canvas>
        </View>
      </GestureDetector>
    </GestureHandlerRootView>
  );
}
