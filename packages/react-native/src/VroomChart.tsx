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
import { ease, easingIndex } from './easing';
import type { VroomChartProps } from './types';
import './jsi.d';

/**
 * Skia-rendered candlestick chart. Pass OHLCV `candles` and size it via `style`
 * (it fills its parent by default). Pan to scroll, pinch to zoom, drag the
 * price/time axes to rescale, and long-press for the crosshair. Optional
 * indicators (`rsi`, `macd`, `movingAverages`, `vwap`), colors (`theme`), and
 * events (`onCrosshair`, `onViewportChange`) are configured through props.
 *
 * @see {@link VroomChartProps} for the full prop reference.
 */
export function VroomChart(props: VroomChartProps) {
  const {
    candles,
    width: widthProp,
    height: heightProp,
    style,
    visibleRange,
    defaultCandleWidth,
    chartType,
    transitionMs,
    transitionEasing,
    theme,
    rsi,
    macd,
    movingAverages,
    vwap,
    bollingerBands,
    volume,
    crosshairOffset = 40,
    onCrosshair,
    onViewportChange,
    priceLines,
    priceLinesStyle,
    onPriceLineDrag,
    onPriceLineDragEnd,
    onPriceLineClose,
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

  // The close button is callback-gated, so whether a handler exists is part of
  // what gets rendered.
  const priceLinesProp = useMemo(
    () =>
      priceLines
        ? {
            lines: priceLines,
            style: priceLinesStyle,
            hasCloseHandler: onPriceLineClose != null,
          }
        : undefined,
    [priceLines, priceLinesStyle, onPriceLineClose],
  );

  const { handle, picture, volumeCollapseRef } = useChartCore(
    candles,
    { width, height },
    visibleRange,
    defaultCandleWidth,
    chartType,
    theme,
    rsi,
    macd,
    movingAverages,
    vwap,
    bollingerBands,
    volume,
    priceLinesProp,
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

  // Candle↔line morph. When `chartType` changes we drive the core per-frame with
  // a (collapse, fade) blend and push a fresh picture into the SV each frame — the
  // JS side owns the eased clock (see plan). A fresh handle snaps to the target;
  // transitionMs=0 snaps. Mirrors the web driver in react/src/useChartCore.ts.
  const morphRaf = useRef<number | null>(null);
  const morphFade = useRef<number | null>(null);
  const morphHandle = useRef<typeof handle>(null);
  // In a ref so changing the curve mid-animation doesn't restart the clock.
  const easingRef = useRef(transitionEasing);
  easingRef.current = transitionEasing;
  useEffect(() => {
    if (!handle) return undefined;
    const target = chartType === 'line' ? 1 : 0;

    // Fresh handle (first load / recreate): snap, don't animate.
    if (morphHandle.current !== handle || morphFade.current == null) {
      morphHandle.current = handle;
      morphFade.current = target;
      handle.setChartType(target);
      const p = handle.render();
      if (p) pictureSV.value = p;
      return undefined;
    }
    if (morphFade.current === target) return undefined;

    if (morphRaf.current != null) {
      cancelAnimationFrame(morphRaf.current);
      morphRaf.current = null;
    }
    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0) {
      morphFade.current = target;
      handle.setChartType(target);
      const p = handle.render();
      if (p) pictureSV.value = p;
      return undefined;
    }

    const from = morphFade.current;
    let startTs: number | null = null;
    const step = (now: number) => {
      if (startTs == null) startTs = now;
      const prog = Math.min(1, (now - startTs) / dur);
      const fade = from + (target - from) * ease(easingRef.current, prog);
      morphFade.current = fade;
      handle.setMorph(fade, fade);
      const p = handle.render();
      if (p) pictureSV.value = p;
      if (prog < 1) {
        morphRaf.current = requestAnimationFrame(step);
      } else {
        morphRaf.current = null;
        morphFade.current = target;
        handle.setChartType(target); // lock the exact endpoint
        const q = handle.render();
        if (q) pictureSV.value = q;
      }
    };
    morphRaf.current = requestAnimationFrame(step);

    return () => {
      if (morphRaf.current != null) {
        cancelAnimationFrame(morphRaf.current);
        morphRaf.current = null;
      }
    };
  }, [handle, chartType, transitionMs, pictureSV]);

  // Volume-bar collapse. The core staggers the bars itself — tallest falling
  // first, all landing together — so unlike the loop above this one hands it
  // *linear* progress plus the curve; pre-easing here would compound the two.
  // Hiding drives 0→1, revealing 1→0, which is the same cascade backwards.
  // Mirrors the web driver in react/src/useChartCore.ts.
  const volumeRaf = useRef<number | null>(null);
  const volumeHandle = useRef<typeof handle>(null);
  useEffect(() => {
    if (!handle) return undefined;
    const target = (volume?.enabled ?? true) ? 0 : 1;
    const easing = easingIndex(easingRef.current);

    // Fresh handle (first load / recreate): the data effect's setVolume already
    // snapped it, so a chart that mounts with bars doesn't animate them in.
    if (volumeHandle.current !== handle || volumeCollapseRef.current == null) {
      volumeHandle.current = handle;
      volumeCollapseRef.current = { t: target, easing };
      return undefined;
    }
    if (volumeCollapseRef.current.t === target) return undefined;

    if (volumeRaf.current != null) {
      cancelAnimationFrame(volumeRaf.current);
      volumeRaf.current = null;
    }

    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0) {
      volumeCollapseRef.current = { t: target, easing };
      handle.setVolumeCollapse(target, easing);
      const p = handle.render();
      if (p) pictureSV.value = p;
      return undefined;
    }

    // From wherever the last frame left off, so toggling mid-flight reverses
    // instead of jumping. A partial trip covers less ground in the same time.
    const from = volumeCollapseRef.current.t;
    let startTs: number | null = null;
    const step = (now: number) => {
      if (startTs == null) startTs = now;
      const prog = Math.min(1, (now - startTs) / dur);
      const t = prog < 1 ? from + (target - from) * prog : target;
      const kind = easingIndex(easingRef.current);
      volumeCollapseRef.current = { t, easing: kind };
      handle.setVolumeCollapse(t, kind);
      const p = handle.render();
      if (p) pictureSV.value = p;
      volumeRaf.current = prog < 1 ? requestAnimationFrame(step) : null;
    };
    volumeRaf.current = requestAnimationFrame(step);

    return () => {
      if (volumeRaf.current != null) {
        cancelAnimationFrame(volumeRaf.current);
        volumeRaf.current = null;
      }
    };
  }, [handle, volume?.enabled, transitionMs, pictureSV, volumeCollapseRef]);

  // Classifies a touch point into the candle area vs. an axis strip. Axis
  // strips always own their gesture (scale price/time) and take priority over
  // the crosshair: an axis touch never opens, moves, or dismisses it.
  const hitAxis = useCallback(
    (x: number, y: number): 'chart' | 'price-axis' | 'time-axis' | 'indicator' => {
      if (!handle) return 'chart';
      const { yAxisWidth, xAxisHeight, indicatorHeight } =
        handle.getAxisMetrics();
      if (x > width - yAxisWidth) return 'price-axis';
      if (y > height - xAxisHeight) return 'time-axis';
      // The indicator pane sits just above the time-axis strip. A drag here
      // scrolls the candles horizontally (no vertical price change).
      if (indicatorHeight > 0 && y > height - xAxisHeight - indicatorHeight) {
        return 'indicator';
      }
      return 'chart';
    },
    [handle, width, height],
  );

  // Hit-tests the price lines at a touch point, resolving the core's index back
  // to the line it belongs to. Null when nothing was hit.
  const hitPriceLine = useCallback(
    (x: number, y: number) => {
      if (!handle || !priceLines?.length) return null;
      const hit = handle.hitTestPriceLine(x, y);
      const line = hit ? priceLines[hit.index] : undefined;
      return hit && line ? { index: hit.index, part: hit.part, line } : null;
    },
    [handle, priceLines],
  );

  // A price line being dragged vertically: its core index, its id, and the last
  // previewed price (the payload for the drop).
  const priceDrag = useRef<{ index: number; id: string; price: number } | null>(
    null,
  );

  // Pan routes to different C++ mutators depending on where it started: the
  // candle area (chart scroll / crosshair move), the y-axis strip (price
  // scale), the x-axis strip (time scale), the indicator pane (horizontal
  // scroll only), or a draggable price line. We classify on onStart.
  const panMode = useRef<
    'chart' | 'price-axis' | 'time-axis' | 'indicator' | 'price-line'
  >('chart');

  const pan = Gesture.Pan()
    .runOnJS(true)
    .maxPointers(1)  // don't fight Pinch's two-finger gesture
    .onStart((e) => {
      cancelDecay();
      // Always classify — an axis drag controls the axis even while the
      // crosshair is up. Only a chart-area drag interacts with the crosshair.
      panMode.current = hitAxis(e.x, e.y);
      // A draggable price line takes the drag over from the chart. Seeding the
      // preview at the committed price puts the label in drag styling before the
      // first move, so the grab registers immediately.
      priceDrag.current = null;
      if (handle && panMode.current === 'chart' && !crosshairActive.current) {
        const pl = hitPriceLine(e.x, e.y);
        if (pl && pl.part === 0) {
          panMode.current = 'price-line';
          priceDrag.current = { index: pl.index, id: pl.line.id, price: pl.line.price };
          handle.setPriceLineDrag(pl.index, pl.line.price);
          const p = handle.render();
          if (p) pictureSV.value = p;
        }
      }
    })
    .onChange((e) => {
      if (!handle) return;
      let next: ReturnType<typeof handle.pan> = null;
      if (panMode.current === 'price-axis') {
        next = handle.scalePriceAxis(e.changeY);
      } else if (panMode.current === 'time-axis') {
        next = handle.scaleTimeAxis(e.changeX);
      } else if (panMode.current === 'indicator') {
        // Drag in an indicator pane scrolls the candles horizontally only —
        // no vertical price slide (the pane's scale is fixed).
        next = handle.pan(e.changeX, 0);
      } else if (panMode.current === 'price-line') {
        // Preview the price under the finger; nothing is committed until the drop.
        const g = priceDrag.current;
        if (!g) return;
        const c = handle.coordAt(e.x, e.y);
        if (!c) return;
        g.price = c.price;
        handle.setPriceLineDrag(g.index, c.price);
        onPriceLineDrag?.(g.id, c.price);
        next = handle.render();
      } else if (crosshairActive.current) {
        // Chart area + crosshair up → the drag moves the crosshair instead of
        // scrolling. Vertical line tracks the finger x; the dot/horizontal line
        // stay lifted `crosshairOffset` px above the fingertip.
        const ch = handle.setCrosshair(e.x, e.y - crosshairOffset);
        if (ch) pictureSV.value = ch;
        // The line follows the finger every frame (above), but only notify the
        // host when the snapped slot actually changes. The slot has a timeMs
        // even in the empty space ahead of the last candle, where candle=null.
        const info = handle.getCrosshairInfo();
        const t = info?.timeMs ?? null;
        if (t !== lastCrosshairTime.current) {
          lastCrosshairTime.current = t;
          // price is web-only for now (see @vroomchart/react); RN reports null.
          onCrosshair?.({ active: true, candle: info?.candle ?? null, timeMs: t, price: null, reason: 'move' });
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
      // Price-line drop. The preview always clears here: the line is a controlled
      // prop, so it only really moves once the host restates it — which means a
      // rejected (or ignored) move reverts on its own.
      if (panMode.current === 'price-line') {
        const g = priceDrag.current;
        priceDrag.current = null;
        handle.setPriceLineDrag(-1, 0);
        const p = handle.render();
        if (p) pictureSV.value = p;
        if (g) onPriceLineDragEnd?.(g.id, g.price);
        return;
      }
      // A chart-area drag with the crosshair up just moved the crosshair —
      // nothing about the viewport changed, and no momentum.
      if (panMode.current === 'chart' && crosshairActive.current) return;
      onViewportChange?.(0, 0);

      // Axis drags don't get momentum — they're a precise size adjustment.
      // Chart and indicator-pane drags both get horizontal fling momentum.
      if (panMode.current !== 'chart' && panMode.current !== 'indicator') return;

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
      // A press on a price line belongs to that line — dragging it or tapping its
      // close button — so it must not raise the crosshair over the top.
      if (hitPriceLine(e.x, e.y)) return;
      cancelDecay();
      crosshairActive.current = true;
      const ch = handle.setCrosshair(e.x, e.y - crosshairOffset);
      if (ch) pictureSV.value = ch;
      const info = handle.getCrosshairInfo();
      lastCrosshairTime.current = info?.timeMs ?? null;
      onCrosshair?.({
        active: true,
        candle: info?.candle ?? null,
        timeMs: info?.timeMs ?? null,
        price: null, // web-only for now (see @vroomchart/react)
        reason: 'show',
      });
    });

  // A tap activates a price line's close button, and otherwise dismisses the
  // crosshair while it's up. Any other tap is a no-op, so it never interferes
  // with normal pan/pinch.
  const tap = Gesture.Tap()
    .runOnJS(true)
    .onStart((e) => {
      if (!handle) return;
      // The close button is a tap target whether or not the crosshair is up.
      const pl = hitPriceLine(e.x, e.y);
      if (pl && pl.part === 1) {
        onPriceLineClose?.(pl.line.id);
        return;
      }
      if (!crosshairActive.current) return;
      // A tap on an axis strip controls the axis, never dismisses the crosshair.
      if (hitAxis(e.x, e.y) !== 'chart') return;
      crosshairActive.current = false;
      const ch = handle.clearCrosshair();
      if (ch) pictureSV.value = ch;
      lastCrosshairTime.current = null;
      onCrosshair?.({ active: false, candle: null, timeMs: null, price: null, reason: 'hide' });
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
