// Pointer/wheel gesture controller for the web chart. Maps input to the same
// VroomChartHandle mutators the React Native gestures use (see
// packages/react-native/src/VroomChart.tsx), so behavior matches across
// platforms. Listeners are attached natively (not via React props) so wheel can
// be non-passive and call preventDefault.

import { useEffect, useRef } from 'react';
import type { ChartMode, CrosshairEvent, Drawing, DrawTool } from '@vroomchart/types';
import type { VroomChartHandle } from '@vroomchart/core-wasm';

type Region = 'chart' | 'price-axis' | 'time-axis' | 'indicator' | 'separator' | 'indicator-axis';

export type GestureOptions = {
  crosshairOffset: number;
  /** Interaction mode. In 'draw' mode panning/zooming/crosshair are suppressed. */
  mode?: ChartMode;
  /** Active drawing tool while in 'draw' mode. */
  tool?: DrawTool;
  onCrosshair?: (e: CrosshairEvent) => void;
  /** External crosshair to mirror (data space), or null. See VroomChartCoreProps. */
  crosshairOverride?: { timeMs: number; price: number } | null;
  onViewportChange?: (startMs: number, endMs: number) => void;
  /** Fired with the finished line when the user places its second point. */
  onDrawingComplete?: (drawing: Drawing) => void;
  /** Fired when the chart wants the host to change mode (e.g. exit on click-away). */
  onRequestMode?: (mode: ChartMode) => void;
};

const MIN_SPAN = 24; // px — minimum two-finger span for an axis to scale
const AXIS_RATIO = 0.5; // an axis scales only if its span ≥ this × the other's
const LONG_PRESS_MS = 350;
const MOVE_THRESH = 6; // px before a press becomes a drag
const WHEEL_K = 0.0015; // wheel delta → zoom factor exponent
const SEP_HIT = 4; // px band around the indicator separator for hit-testing

// Drawing-tool styling: the guideline (and committed line) default to solid blue
// at 2px, matching the core's default and useChartCore's drawing color.
const DRAW_COLOR = 0xff2962ff;
const DRAW_WIDTH = 2;
const DRAW_HIT = 6; // px tolerance for "clicked on the selected line" vs. away

// Stable unique id for a freshly drawn line.
function drawingId(): string {
  if (typeof crypto !== 'undefined' && typeof crypto.randomUUID === 'function') {
    return crypto.randomUUID();
  }
  return `draw-${Math.random().toString(36).slice(2)}`;
}

// Distance in px from point (px,py) to the segment (ax,ay)-(bx,by).
function distToSegment(
  px: number,
  py: number,
  ax: number,
  ay: number,
  bx: number,
  by: number,
): number {
  const dx = bx - ax;
  const dy = by - ay;
  const len2 = dx * dx + dy * dy;
  let t = len2 > 0 ? ((px - ax) * dx + (py - ay) * dy) / len2 : 0;
  t = Math.max(0, Math.min(1, t));
  return Math.hypot(px - (ax + t * dx), py - (ay + t * dy));
}

export function useGestures(
  containerRef: React.RefObject<HTMLElement | null>,
  handleRef: React.RefObject<VroomChartHandle | null>,
  scheduleRender: () => void,
  opts: GestureOptions,
): void {
  // Keep latest opts in a ref so the effect's listeners stay stable.
  const optsRef = useRef(opts);
  optsRef.current = opts;

  // Drawing-tool state. Lives in refs (not effect-local) so the mode-change
  // effect below can reset it when the host leaves draw mode, and so it survives
  // the gesture effect's stable-listener lifecycle.
  //   drawAnchor* — first point placed, awaiting the second (data + px coords).
  //   drawSelectedPx — set once a line is committed: its px endpoints, used to
  //                    hit-test "clicked on the line" vs. "clicked away to exit".
  const drawAnchorRef = useRef<{ timeMs: number; price: number } | null>(null);
  const drawAnchorPxRef = useRef<{ x: number; y: number } | null>(null);
  const drawSelectedPxRef = useRef<{
    ax: number;
    ay: number;
    bx: number;
    by: number;
  } | null>(null);

  // True while a local pointer (hover/press) owns the crosshair. Lets the
  // crosshair-override effect below know to stand down (local input wins).
  const localCrosshairActiveRef = useRef(false);

  // Reset the in-progress draft whenever draw+line mode is not active (e.g. the
  // host toggled the tool off mid-draw), so re-entering draw mode starts clean.
  useEffect(() => {
    const active = opts.mode === 'draw' && opts.tool === 'line';
    if (active) return;
    drawAnchorRef.current = null;
    drawAnchorPxRef.current = null;
    drawSelectedPxRef.current = null;
    const h = handleRef.current;
    if (h) {
      h.clearDraft();
      scheduleRender();
    }
  }, [opts.mode, opts.tool, handleRef, scheduleRender]);

  // Apply the controlled crosshair override (cross-chart sync). A local
  // hover/press always wins, so we stand down while one is active; the override
  // is (re)applied on local release inside hideCrosshair. This path is silent —
  // it never calls onCrosshair — so two charts driving each other can't loop.
  // (If the override is set before the WASM handle finishes loading it applies
  // on its next change; in the sync use case the initial value is null.)
  useEffect(() => {
    const h = handleRef.current;
    if (!h || localCrosshairActiveRef.current) return;
    const ov = opts.crosshairOverride;
    if (ov) h.setCrosshairData(ov.timeMs, ov.price);
    else h.clearCrosshair();
    scheduleRender();
  }, [opts.crosshairOverride, handleRef, scheduleRender]);

  useEffect(() => {
    const el = containerRef.current;
    if (!el) return;

    const pointers = new Map<number, { x: number; y: number }>();
    let panMode: Region = 'chart';
    let crosshairActive = false;
    let crosshairSource: 'press' | 'hover' | null = null;
    let lastCrosshairTime: number | null = null;
    let lastCrosshairPrice: number | null = null;
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
      if (x > r.width - yAxisWidth) {
        // The y-axis strip beside an indicator pane scales that pane's y-axis;
        // the rest of the strip scales the main price axis.
        const priceBottom = r.height - xAxisHeight - indicatorHeight;
        if (indicatorHeight > 0 && y > priceBottom && y < r.height - xAxisHeight) {
          return 'indicator-axis';
        }
        return 'price-axis';
      }
      // Separator sits at the top edge of the indicator band, within the candle
      // area width — a narrow grab band for resizing the panes.
      const sepY = r.height - xAxisHeight - indicatorHeight;
      if (indicatorHeight > 0 && x <= r.width - yAxisWidth && Math.abs(y - sepY) <= SEP_HIT) {
        return 'separator';
      }
      if (y > r.height - xAxisHeight) return 'time-axis';
      if (indicatorHeight > 0 && y > r.height - xAxisHeight - indicatorHeight) return 'indicator';
      return 'chart';
    };

    const reportCrosshair = (reason: CrosshairEvent['reason']) => {
      const h = handleRef.current;
      if (!h) return;
      // Snapped slot — has a timeMs even in the empty space ahead of the last
      // candle, where `candle` is null.
      const info = h.getCrosshairInfo();
      const t = info?.timeMs ?? null;
      const p = info?.price ?? null;
      // Fire on any positional change — a different candle (time) OR a vertical
      // move within the same candle (price). Deduping on time alone would freeze
      // the reported price mid-candle, which breaks cross-chart price sync.
      if (reason === 'move' && t === lastCrosshairTime && p === lastCrosshairPrice) return;
      lastCrosshairTime = t;
      lastCrosshairPrice = p;
      optsRef.current.onCrosshair?.({
        active: reason !== 'hide',
        candle: info?.candle ?? null,
        timeMs: t,
        price: reason === 'hide' ? null : p,
        reason,
      });
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
      localCrosshairActiveRef.current = true;
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
      localCrosshairActiveRef.current = false;
      crosshairSource = null;
      // Fall back to the synced crosshair (if any) rather than clearing, so
      // releasing a local hover on a follower chart restores the driver's line.
      const ov = optsRef.current.crosshairOverride;
      if (ov) h.setCrosshairData(ov.timeMs, ov.price);
      else h.clearCrosshair();
      scheduleRender();
      reportCrosshair('hide');
    };

    // True while the line tool should own input (suppress pan/zoom/crosshair).
    const drawActive = () =>
      optsRef.current.mode === 'draw' && optsRef.current.tool === 'line';

    // While placing the second point, keep the guideline glued to the cursor.
    const updateGuideline = (x: number, y: number) => {
      const h = handleRef.current;
      if (!h) return;
      if (!drawAnchorRef.current || drawSelectedPxRef.current) return;
      const c = h.coordAt(x, y);
      if (!c) return;
      const a = drawAnchorRef.current;
      h.setDraft(a.timeMs, a.price, true, c.timeMs, c.price, true, DRAW_COLOR, DRAW_WIDTH);
      scheduleRender();
    };

    // A tap in the chart area: place the next point, or exit on click-away.
    const handleDrawClick = (x: number, y: number) => {
      const h = handleRef.current;
      if (!h) return;
      const o = optsRef.current;

      // A committed line is selected: clicking on it keeps it; clicking away
      // hides the nodes and asks the host to return to pan mode.
      const sel = drawSelectedPxRef.current;
      if (sel) {
        if (distToSegment(x, y, sel.ax, sel.ay, sel.bx, sel.by) <= DRAW_HIT) return;
        drawAnchorRef.current = null;
        drawAnchorPxRef.current = null;
        drawSelectedPxRef.current = null;
        h.clearDraft();
        scheduleRender();
        el.style.cursor = '';
        o.onRequestMode?.('pan');
        return;
      }

      const coord = h.coordAt(x, y);
      if (!coord) return;

      if (!drawAnchorRef.current) {
        // First point: show node A; the guideline follows on the next move.
        drawAnchorRef.current = coord;
        drawAnchorPxRef.current = { x, y };
        h.setDraft(coord.timeMs, coord.price, false, 0, 0, true, DRAW_COLOR, DRAW_WIDTH);
        scheduleRender();
      } else {
        // Second point: commit the line and keep both nodes shown (selected).
        const a = drawAnchorRef.current;
        const apx = drawAnchorPxRef.current!;
        o.onDrawingComplete?.({
          id: drawingId(),
          type: 'line',
          points: [
            { timeMs: a.timeMs, price: a.price },
            { timeMs: coord.timeMs, price: coord.price },
          ],
        });
        drawSelectedPxRef.current = { ax: apx.x, ay: apx.y, bx: x, by: y };
        h.setDraft(a.timeMs, a.price, true, coord.timeMs, coord.price, false, DRAW_COLOR, DRAW_WIDTH);
        scheduleRender();
      }
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
      if (e.pointerType !== 'mouse' && panMode === 'chart' && !drawActive()) {
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
          // Draw mode: no crosshair; the guideline tracks the cursor instead.
          if (drawActive()) {
            el.style.cursor = 'crosshair';
            updateGuideline(x, y);
            return;
          }
          const region = regionAt(x, y);
          el.style.cursor =
            region === 'separator'
              ? 'row-resize'
              : region === 'indicator-axis'
                ? 'ns-resize'
                : '';
          if (region === 'chart') {
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

      // Two-finger directional pinch (disabled while drawing).
      if (pointers.size >= 2 && pinch.active && !drawActive()) {
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

      // Draw mode: a press-drag neither pans nor moves a crosshair; if a point
      // is down, keep the guideline tracking the cursor so a drag still previews.
      if (drawActive()) {
        if (panMode === 'chart') updateGuideline(x, y);
        return;
      }

      // Chart-area drag with the (press) crosshair up moves the crosshair.
      if (crosshairActive && crosshairSource === 'press' && panMode === 'chart') {
        showCrosshair(x, y, 'press', 'move');
        return;
      }

      if (panMode === 'price-axis') h.scalePriceAxis(dy);
      else if (panMode === 'time-axis') h.scaleTimeAxis(dx);
      else if (panMode === 'separator') h.resizeIndicatorPane(dy);
      else if (panMode === 'indicator-axis') h.scaleIndicatorAxis(downY, dy);
      else if (panMode === 'indicator') h.pan(dx, 0);
      else h.translate(dx, dy);

      // Keep a hover crosshair glued to the cursor while dragging — the chart
      // pans under it, so re-snap to whatever candle is now beneath the cursor.
      if (crosshairActive && crosshairSource === 'hover') {
        h.setCrosshair(x, y);
        reportCrosshair('move');
      }
      scheduleRender();
    };

    const endPointer = (e: PointerEvent) => {
      const had = pointers.delete(e.pointerId);
      clearLongPress();
      if (pointers.size < 2) pinch.active = false;
      if (!had) return;

      // Draw mode: a stationary tap in the chart places a point (or exits on
      // click-away). Drags are ignored (no pan). Crosshair logic is skipped.
      if (drawActive() && pointers.size === 0) {
        if (!moved && panMode === 'chart') handleDrawClick(downX, downY);
        else el.style.cursor = 'crosshair';
        return;
      }

      // Press crosshair is active only while held — release dismisses it.
      if (crosshairSource === 'press' && pointers.size === 0) {
        hideCrosshair();
      } else if (moved && (panMode === 'chart' || panMode === 'indicator')) {
        optsRef.current.onViewportChange?.(0, 0);
      }
      if (pointers.size === 0) el.style.cursor = '';
    };

    const onPointerLeave = () => {
      if (crosshairSource === 'hover') hideCrosshair();
      el.style.cursor = '';
    };

    const onWheel = (e: WheelEvent) => {
      const h = handleRef.current;
      if (!h) return;
      e.preventDefault();
      // Draw mode: no pan/zoom from the wheel (keeps placed points stable).
      if (drawActive()) return;
      const { x, y } = rel(e);
      if (e.ctrlKey || e.metaKey) {
        // Trackpad pinch (sent as ctrl+wheel) / ctrl+wheel → zoom both axes.
        const f = Math.exp(-e.deltaY * WHEEL_K);
        h.zoom(f, f, x, y);
      } else if (e.shiftKey) {
        // Shift+wheel → horizontal pan (mouse-wheel-only users).
        h.pan(-(e.deltaX || e.deltaY), 0);
      } else if (Math.abs(e.deltaX) > Math.abs(e.deltaY)) {
        // Horizontal scroll → pan the time axis (scroll right = forward in time).
        h.pan(-e.deltaX, 0);
      } else {
        // Vertical scroll → zoom the time window around the cursor.
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
