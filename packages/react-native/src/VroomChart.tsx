// VroomChart — Phase 3.
//
// Owns SharedValues driven by:
//   - useChartCore's "initial" frame (when data/size/range change), AND
//   - Pan gesture callbacks that call handle.pan(dx, dy) → a fresh frame.
//
// iOS wraps an SkPicture in-process. Android rasterizes to an SkImage (the
// two Skia copies can't share a picture pointer) so pan/zoom don't serialize
// the scene — and the system typeface — on every frame.
//
// Reanimated 4 + RN-Skia 2 propagate SharedValue changes to <Picture>/<Image>
// without a React re-render, so gesture-driven redraws are cheap.
//
// Gestures run on the JS thread for now (`runOnJS(true)`) — installing the
// JSI bindings on the worklet runtime is a later perf optimization.

import React, { useEffect, useRef, useCallback, useState, useMemo } from 'react';
import { PixelRatio, View, type LayoutChangeEvent } from 'react-native';
import {
  AlphaType,
  Canvas,
  ColorType,
  Image,
  Picture,
  Skia,
  type SkImage,
  type SkPicture,
} from '@shopify/react-native-skia';
import {
  Gesture,
  GestureDetector,
  GestureHandlerRootView,
} from 'react-native-gesture-handler';
import { useReducedMotion, useSharedValue } from 'react-native-reanimated';

import { useChartCore } from './useChartCore';
import { ease, easingIndex } from './easing';
import type { ChartFrame } from './jsi.d';
import type { VroomChartProps } from './types';
import './jsi.d';

function isSkImage(frame: ChartFrame): frame is SkImage {
  return typeof (frame as SkImage).getImageInfo === 'function';
}

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
    seriesKey,
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

  // RN-Skia's recorder reads these SharedValues on the UI/render runtime, a
  // beat behind JS-thread writes. If it ever reads null it throws ("Invalid
  // prop value for SkTextBlob received" — RN-Skia's mislabeled SkPicture
  // error), so we seed them and *never* assign null. Android writes the image
  // SV (raster path); iOS writes the picture SV. The unused layer stays a
  // transparent 1×1 so it doesn't cover the other.
  const emptyPicture = useMemo(() => {
    const rec = Skia.PictureRecorder();
    rec.beginRecording(Skia.XYWHRect(0, 0, 1, 1));
    return rec.finishRecordingAsPicture();
  }, []);
  const emptyImage = useMemo(() => {
    const data = Skia.Data.fromBytes(new Uint8Array(4));
    return Skia.Image.MakeImage(
      {
        width: 1,
        height: 1,
        colorType: ColorType.RGBA_8888,
        alphaType: AlphaType.Premul,
      },
      data,
      4,
    )!;
  }, []);
  const pictureSV = useSharedValue<SkPicture>(emptyPicture);
  const imageSV = useSharedValue<SkImage>(emptyImage);
  const applyFrame = useCallback(
    (frame: ChartFrame) => {
      if (isSkImage(frame)) imageSV.value = frame;
      else pictureSV.value = frame;
    },
    [imageSV, pictureSV],
  );

  // An OS reduced-motion preference snaps every transition, the way
  // prefers-reduced-motion does on web.
  const reduceMotion = useReducedMotion();

  // The interval morph starts inside the data effect (it needs the pre-swap
  // capture) but repaints every frame, so it writes straight into the SV rather
  // than through React state — the same bypass the gesture handlers use.
  const onFrame = useCallback(
    (p: ChartFrame) => {
      applyFrame(p);
    },
    [applyFrame],
  );

  const { handle, picture, volumeCollapseRef } = useChartCore(
    candles,
    { width, height, pxRatio: PixelRatio.get() },
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
    { seriesKey, transitionMs, transitionEasing, reduceMotion, onFrame },
  );

  // When the crosshair is showing, pan moves it (instead of scrolling) and
  // pinch is disabled. A ref (not state) so gesture callbacks read it
  // synchronously without re-subscribing. Tap dismisses it.
  const crosshairActive = useRef(false);

  // timeMs of the candle last reported through onCrosshair, so a drag fires a
  // 'move' event only when it crosses into a *different* candle (one per
  // candle, not per frame). Null while the crosshair is hidden.
  const lastCrosshairTime = useRef<number | null>(null);

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
    if (next) applyFrame(next);
    if (handle.isAnimating()) {
      animRaf.current = requestAnimationFrame(animTick);
    }
  }, [handle, applyFrame]);
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

  // Sync the initial picture from useChartCore into the SV whenever it
  // refreshes (data load, size change, externally-controlled range change).
  // Only ever assign a non-null picture (see emptyPicture note above).
  //
  // Sits below maybeStartAnim so it can kick the loop: it's the only path a
  // non-gesture change has into it, which is what a theme that turns the line-tip
  // pulse on needs — otherwise the ring wouldn't move until you touched the
  // chart.
  useEffect(() => {
    if (picture) applyFrame(picture);
    maybeStartAnim();
  }, [picture, applyFrame, maybeStartAnim]);

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

    // Every exit below hands off to maybeStartAnim. Landing in line mode turns
    // tip_pulse_active() on, and the pulse only moves while that loop is
    // requeueing frames — this loop's own clock stops here. Without the handoff
    // the ring sits frozen until some gesture happens to restart the other loop.
    // It no-ops when a frame is already queued or nothing is animating, so
    // landing in candle mode costs nothing.

    // Fresh handle (first load / recreate): snap, don't animate.
    if (morphHandle.current !== handle || morphFade.current == null) {
      morphHandle.current = handle;
      morphFade.current = target;
      handle.setChartType(target);
      const p = handle.render();
      if (p) applyFrame(p);
      maybeStartAnim();
      return undefined;
    }
    if (morphFade.current === target) {
      maybeStartAnim();
      return undefined;
    }

    if (morphRaf.current != null) {
      cancelAnimationFrame(morphRaf.current);
      morphRaf.current = null;
    }
    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0) {
      morphFade.current = target;
      handle.setChartType(target);
      const p = handle.render();
      if (p) applyFrame(p);
      maybeStartAnim();
      return undefined;
    }

    const from = morphFade.current;
    let startTs: number | null = null;
    const step = (now: number) => {
      if (startTs == null) startTs = now;
      const prog = Math.min(1, (now - startTs) / dur);
      const fade = from + (target - from) * ease(easingRef.current, prog);
      morphFade.current = fade;
      // Reduced motion still crossfades, but skips the vertical collapse.
      handle.setMorph(reduceMotion ? 0 : fade, fade);
      const p = handle.render();
      if (p) applyFrame(p);
      if (prog < 1) {
        morphRaf.current = requestAnimationFrame(step);
      } else {
        morphRaf.current = null;
        morphFade.current = target;
        handle.setChartType(target); // lock the exact endpoint
        const q = handle.render();
        if (q) applyFrame(q);
        maybeStartAnim();
      }
    };
    morphRaf.current = requestAnimationFrame(step);

    return () => {
      if (morphRaf.current != null) {
        cancelAnimationFrame(morphRaf.current);
        morphRaf.current = null;
      }
    };
    // maybeStartAnim is memoized on [handle, animTick] and animTick on
    // [handle, applyFrame], both already deps here — so it adds no new restarts
    // of this clock.
  }, [handle, chartType, transitionMs, reduceMotion, applyFrame, maybeStartAnim]);

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
    // Every exit below hands off to maybeStartAnim, for the same reason the
    // morph loop does: this clock stops here, and anything the core is still
    // animating (the line-tip pulse) needs the other loop requeueing frames.
    if (volumeHandle.current !== handle || volumeCollapseRef.current == null) {
      volumeHandle.current = handle;
      volumeCollapseRef.current = { t: target, easing };
      maybeStartAnim();
      return undefined;
    }
    if (volumeCollapseRef.current.t === target) {
      maybeStartAnim();
      return undefined;
    }

    if (volumeRaf.current != null) {
      cancelAnimationFrame(volumeRaf.current);
      volumeRaf.current = null;
    }

    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0 || reduceMotion) {
      volumeCollapseRef.current = { t: target, easing };
      handle.setVolumeCollapse(target, easing);
      const p = handle.render();
      if (p) applyFrame(p);
      maybeStartAnim();
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
      if (p) applyFrame(p);
      if (prog < 1) {
        volumeRaf.current = requestAnimationFrame(step);
      } else {
        volumeRaf.current = null;
        maybeStartAnim();
      }
    };
    volumeRaf.current = requestAnimationFrame(step);

    return () => {
      if (volumeRaf.current != null) {
        cancelAnimationFrame(volumeRaf.current);
        volumeRaf.current = null;
      }
    };
  }, [
    handle,
    volume?.enabled,
    transitionMs,
    reduceMotion,
    applyFrame,
    volumeCollapseRef,
    maybeStartAnim,
  ]);

  // Axis-strip collapse. Unlike the volume bars there is no per-element stagger
  // for the core to distribute, so this pre-eases in JS and hands over the eased
  // scalar. Both axes ride one clock so toggling them together stays in step.
  // This one moves the *layout* — the plot reflows into the reclaimed space
  // every frame. Mirrors the web driver in react/src/useChartCore.ts.
  const axisRaf = useRef<number | null>(null);
  const axisHandle = useRef<typeof handle>(null);
  const axisCollapse = useRef<{ y: number; x: number } | null>(null);
  const showYAxis = theme?.showYAxis ?? true;
  const showXAxis = theme?.showXAxis ?? true;
  useEffect(() => {
    if (!handle) return undefined;
    const targetY = showYAxis ? 0 : 1;
    const targetX = showXAxis ? 0 : 1;

    // Fresh handle (first load / recreate): snap, so a chart that mounts with an
    // axis already hidden doesn't play it out.
    if (axisHandle.current !== handle || axisCollapse.current == null) {
      axisHandle.current = handle;
      axisCollapse.current = { y: targetY, x: targetX };
      handle.setAxisCollapse(targetY, targetX);
      const p = handle.render();
      if (p) applyFrame(p);
      maybeStartAnim();
      return undefined;
    }

    const fromY = axisCollapse.current.y;
    const fromX = axisCollapse.current.x;
    if (fromY === targetY && fromX === targetX) {
      maybeStartAnim();
      return undefined;
    }

    if (axisRaf.current != null) {
      cancelAnimationFrame(axisRaf.current);
      axisRaf.current = null;
    }

    const dur = Math.max(0, transitionMs ?? 300);
    if (dur === 0 || reduceMotion) {
      axisCollapse.current = { y: targetY, x: targetX };
      handle.setAxisCollapse(targetY, targetX);
      const p = handle.render();
      if (p) applyFrame(p);
      maybeStartAnim();
      return undefined;
    }

    // From wherever the last frame left off, so toggling mid-flight reverses
    // instead of jumping.
    let startTs: number | null = null;
    const step = (now: number) => {
      if (startTs == null) startTs = now;
      const prog = Math.min(1, (now - startTs) / dur);
      const e = ease(easingRef.current, prog);
      const y = prog < 1 ? fromY + (targetY - fromY) * e : targetY;
      const x = prog < 1 ? fromX + (targetX - fromX) * e : targetX;
      axisCollapse.current = { y, x };
      handle.setAxisCollapse(y, x);
      const p = handle.render();
      if (p) applyFrame(p);
      if (prog < 1) {
        axisRaf.current = requestAnimationFrame(step);
      } else {
        axisRaf.current = null;
        maybeStartAnim();
      }
    };
    axisRaf.current = requestAnimationFrame(step);

    return () => {
      if (axisRaf.current != null) {
        cancelAnimationFrame(axisRaf.current);
        axisRaf.current = null;
      }
    };
  }, [
    handle,
    showYAxis,
    showXAxis,
    transitionMs,
    reduceMotion,
    applyFrame,
    maybeStartAnim,
  ]);

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
          if (p) applyFrame(p);
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
        if (ch) applyFrame(ch);
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
      if (next) applyFrame(next);
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
        if (p) applyFrame(p);
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
        if (next) applyFrame(next);
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
      if (next) applyFrame(next);
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
      if (ch) applyFrame(ch);
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
      if (ch) applyFrame(ch);
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
              <>
                <Picture picture={pictureSV} />
                <Image
                  image={imageSV}
                  x={0}
                  y={0}
                  width={width}
                  height={height}
                  fit="fill"
                />
              </>
            ) : null}
          </Canvas>
        </View>
      </GestureDetector>
    </GestureHandlerRootView>
  );
}
