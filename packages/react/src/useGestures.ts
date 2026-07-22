// Pointer/wheel gesture controller for the web chart. Maps input to the same
// VroomChartHandle mutators the React Native gestures use (see
// packages/react-native/src/VroomChart.tsx), so behavior matches across
// platforms. Listeners are attached natively (not via React props) so wheel can
// be non-passive and call preventDefault.

import { useCallback, useEffect, useRef } from 'react';
import type { ChartMode, CrosshairEvent, Drawing, DrawPoint, DrawTool } from '@vroomchart/types';
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
  /** Committed drawings (controlled). Used to hit-test/select/drag/delete. */
  drawings?: Drawing[];
  /** Fired with the finished line when the user places its second point. */
  onDrawingComplete?: (drawing: Drawing) => void;
  /** Fired after a selected line's endpoint is dragged (same id, new points). */
  onDrawingChange?: (drawing: Drawing) => void;
  /** Fired when the selected line is deleted (Backspace/Delete). */
  onDrawingDelete?: (id: string) => void;
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

// Stable unique id for a freshly drawn line.
function drawingId(): string {
  if (typeof crypto !== 'undefined' && typeof crypto.randomUUID === 'function') {
    return crypto.randomUUID();
  }
  return `draw-${Math.random().toString(36).slice(2)}`;
}

// The four corners of a box (given its two opposite-corner `points`), in the
// same order the core's hit-test reports: a, (b.time,a.price), b,
// (a.time,b.price). Corner k's diagonal is corner (k+2)%4.
function boxCorners(
  points: readonly [DrawPoint, DrawPoint],
): [DrawPoint, DrawPoint, DrawPoint, DrawPoint] {
  const [a, b] = points;
  return [
    { timeMs: a.timeMs, price: a.price },
    { timeMs: b.timeMs, price: a.price },
    { timeMs: b.timeMs, price: b.price },
    { timeMs: a.timeMs, price: b.price },
  ];
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

  // Drawing-tool state (live in refs so the mode-change effect can reset it and
  // it survives the gesture effect's stable-listener lifecycle).
  //   drawAnchor — first point placed, awaiting the second (data coords).
  const drawAnchorRef = useRef<{ timeMs: number; price: number } | null>(null);

  // Editing state for committed drawings (pan-mode select / drag / delete).
  //   selectedId — the selected Drawing.id (data-space identity; survives
  //                pan/zoom + list changes). The core is told the matching index.
  //   grab — the endpoint being dragged { index, endpoint: 0|1 }, or null.
  //   dragLine — the whole selected line being translated by its body, or null.
  //   downHit — the hit under the last pointerdown, resolved to select/deselect
  //             on a stationary release.
  //   lastCoord — the last coordAt during a handle drag, for the release payload.
  const selectedIdRef = useRef<string | null>(null);
  // The endpoint/corner being dragged. `fixed` is the anchor that stays put (the
  // other line endpoint, or — for a box — the diagonally opposite corner). For a
  // box we normalize the core so `endpoint` is always 0 (the grabbed corner) and
  // `fixed` is stored as endpoint 1, so the line's endpoint-drag path drives it.
  const grabRef = useRef<{
    index: number;
    endpoint: number;
    fixed: DrawPoint;
    isBox: boolean;
  } | null>(null);
  const dragLineRef = useRef<{
    index: number;
    startTime: number;
    startPrice: number;
    a0: DrawPoint;
    b0: DrawPoint;
    lastA: DrawPoint;
    lastB: DrawPoint;
  } | null>(null);
  const downHitRef = useRef<{ index: number; part: number; t: number } | null>(null);
  const lastCoordRef = useRef<{ timeMs: number; price: number } | null>(null);
  // Where along the selected line it was grabbed (0..1, A→B), and the in-memory
  // clipboard for copy/paste (color/width + the grab t + the crosshair at copy).
  const selectedTRef = useRef(0.5);
  const clipboardRef = useRef<{
    type: Drawing['type'];
    points: [DrawPoint, DrawPoint];
    color?: Drawing['color'];
    width?: number;
    t: number;
    crosshair: { timeMs: number; price: number } | null;
  } | null>(null);

  // Push the current selection (id → core index) and grabbed-handle state into
  // the core. Reads live refs/opts so a captured instance is never stale. Runs
  // after useChartCore's setDrawings (declared earlier in VroomChart) so the
  // index is valid whenever the drawings list changes.
  const applySelection = useCallback(() => {
    const h = handleRef.current;
    if (!h) return;
    const id = selectedIdRef.current;
    const list = optsRef.current.drawings ?? [];
    let index = -1;
    if (id != null) {
      index = list.findIndex((d) => d.id === id);
      if (index < 0) selectedIdRef.current = null;
    }
    h.setSelectedDrawing(index, index >= 0 ? (grabRef.current?.endpoint ?? -1) : -1);
    scheduleRender();
  }, [handleRef, scheduleRender]);

  // Re-assert selection whenever the drawings list changes (add / delete / the
  // just-dragged update), so the core index tracks the selected id.
  useEffect(() => {
    applySelection();
  }, [opts.drawings, applySelection]);

  // Keyboard: delete the selected line (Backspace/Delete) and copy/paste it
  // (Cmd/Ctrl+C / +V). Skipped while focus is in a field / a real text selection.
  useEffect(() => {
    const isEditableTarget = () => {
      const ae = document.activeElement as HTMLElement | null;
      const tag = ae?.tagName;
      return tag === 'INPUT' || tag === 'TEXTAREA' || !!ae?.isContentEditable;
    };
    const findSelected = (): Drawing | null => {
      const id = selectedIdRef.current;
      if (id == null) return null;
      return (optsRef.current.drawings ?? []).find((x) => x.id === id) ?? null;
    };

    // Endpoints for a pasted copy: under the crosshair (intersecting at the same
    // grab t), else offset above/below the original, kept within the viewport.
    const placePaste = (clip: NonNullable<typeof clipboardRef.current>): [DrawPoint, DrawPoint] | null => {
      const h = handleRef.current;
      if (!h) return null;
      const [a, b] = clip.points;
      const t = clip.t;
      const pSel = {
        timeMs: a.timeMs + t * (b.timeMs - a.timeMs),
        price: a.price + t * (b.price - a.price),
      };
      const cur = h.getCrosshairInfo();
      const moved =
        cur != null &&
        (clip.crosshair == null ||
          cur.timeMs !== clip.crosshair.timeMs ||
          cur.price !== clip.crosshair.price);
      if (moved && cur) {
        const dt = cur.timeMs - pSel.timeMs;
        const dp = cur.price - pSel.price;
        return [
          { timeMs: a.timeMs + dt, price: a.price + dp },
          { timeMs: b.timeMs + dt, price: b.price + dp },
        ];
      }
      // Above/below the original. Derive the visible price range + px scale.
      const el = containerRef.current;
      const shift = (dp: number): [DrawPoint, DrawPoint] => [
        { timeMs: a.timeMs, price: a.price + dp },
        { timeMs: b.timeMs, price: b.price + dp },
      ];
      const span = Math.abs(a.price - b.price);
      if (!el) return shift(-(span || 1));
      const rect = el.getBoundingClientRect();
      const m = h.getAxisMetrics();
      const priceBottom = rect.height - m.xAxisHeight - m.indicatorHeight;
      const top = h.coordAt(rect.width / 2, 0);
      const bot = h.coordAt(rect.width / 2, priceBottom);
      if (!top || !bot || priceBottom <= 0 || top.price <= bot.price) return shift(-(span || 1));
      const vTop = top.price; // higher price = screen top
      const vBot = bot.price; // lower price = screen bottom
      const pricePerPx = (vTop - vBot) / priceBottom;
      const GAP_PX = 30;
      const offset = (span / pricePerPx + GAP_PX) * pricePerPx; // clear the original
      const pMin = Math.min(a.price, b.price);
      const pMax = Math.max(a.price, b.price);
      const upFits = pMax + offset <= vTop;
      const downFits = pMin - offset >= vBot;
      let dir: 1 | -1;
      if (upFits && !downFits) dir = 1;
      else if (downFits && !upFits) dir = -1;
      else if (upFits && downFits) {
        dir = vTop - (pMax + offset) >= pMin - offset - vBot ? 1 : -1; // more edge margin
      } else {
        const upVis = Math.min(pMax + offset, vTop) - Math.max(pMin + offset, vBot);
        const dnVis = Math.min(pMax - offset, vTop) - Math.max(pMin - offset, vBot);
        dir = upVis >= dnVis ? 1 : -1; // larger visible portion
      }
      return shift(dir * offset);
    };

    const onKeyDown = (e: KeyboardEvent) => {
      if (isEditableTarget()) return;

      // Mid-draw (first anchor placed, awaiting the second): Escape/Delete/
      // Backspace cancels the in-progress anchor but keeps draw mode active.
      if (
        drawAnchorRef.current &&
        (e.key === 'Escape' || e.key === 'Backspace' || e.key === 'Delete')
      ) {
        e.preventDefault();
        drawAnchorRef.current = null;
        const h = handleRef.current;
        if (h) {
          h.clearDraft();
          scheduleRender();
        }
        return;
      }

      if (e.key === 'Backspace' || e.key === 'Delete') {
        const id = selectedIdRef.current;
        if (id == null) return;
        e.preventDefault();
        optsRef.current.onDrawingDelete?.(id);
        selectedIdRef.current = null;
        grabRef.current = null;
        dragLineRef.current = null;
        const h = handleRef.current;
        if (h) {
          h.setSelectedDrawing(-1, -1);
          scheduleRender();
        }
        return;
      }

      if (!(e.metaKey || e.ctrlKey)) return;
      const key = e.key.toLowerCase();
      if (key === 'c') {
        if (window.getSelection()?.toString()) return; // don't hijack a real text copy
        const d = findSelected();
        if (!d) return;
        e.preventDefault();
        const info = handleRef.current?.getCrosshairInfo() ?? null;
        clipboardRef.current = {
          type: d.type,
          points: [d.points[0], d.points[1]],
          color: d.color,
          width: d.width,
          t: selectedTRef.current,
          crosshair: info ? { timeMs: info.timeMs, price: info.price } : null,
        };
      } else if (key === 'v') {
        const clip = clipboardRef.current;
        if (!clip) return;
        const pts = placePaste(clip);
        if (!pts) return;
        e.preventDefault();
        const id = drawingId();
        optsRef.current.onDrawingComplete?.({
          id,
          type: clip.type,
          points: pts,
          ...(clip.color != null ? { color: clip.color } : {}),
          ...(clip.width != null ? { width: clip.width } : {}),
        });
        // The drawings-change effect selects the new id once it's appended.
        selectedIdRef.current = id;
        selectedTRef.current = clip.t;
      }
    };
    window.addEventListener('keydown', onKeyDown);
    return () => window.removeEventListener('keydown', onKeyDown);
  }, [containerRef, handleRef, scheduleRender]);

  // True while a local pointer (hover/press) owns the crosshair. Lets the
  // crosshair-override effect below know to stand down (local input wins).
  const localCrosshairActiveRef = useRef(false);

  // Reset the in-progress draft whenever no draw tool is active (e.g. the host
  // toggled the tool off, or switched tools, mid-draw), so re-entering draw mode
  // starts clean.
  useEffect(() => {
    const active = opts.mode === 'draw' && (opts.tool === 'line' || opts.tool === 'box');
    if (active) return;
    drawAnchorRef.current = null;
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
    // Shift-constrain state: whether Shift is held, and the last cursor position
    // (so a Shift press/release with the mouse still can re-snap live).
    let shiftHeld = false;
    let lastX = 0;
    let lastY = 0;
    const pinch = { spanX: 1, spanY: 1, ratioX: 1, ratioY: 1, enableX: false, enableY: false, active: false };

    const rel = (e: PointerEvent | WheelEvent) => {
      const r = el.getBoundingClientRect();
      return { x: e.clientX - r.left, y: e.clientY - r.top };
    };

    // Data coord to use for a moving point at pixel (x,y) relative to `fixed`.
    // With Shift held, snaps to the nearest 45° *screen* angle from `fixed`.
    const snapToLine = (
      fixed: DrawPoint,
      x: number,
      y: number,
    ): { timeMs: number; price: number } | null => {
      const h = handleRef.current;
      if (!h) return null;
      const free = h.coordAt(x, y);
      if (!shiftHeld || !free) return free;
      const f = h.project(fixed.timeMs, fixed.price);
      if (!f) return free;
      const dx = x - f.x;
      const dy = y - f.y;
      if (dx === 0 && dy === 0) return free;
      const step = Math.PI / 4;
      const ang = Math.round(Math.atan2(dy, dx) / step) * step; // nearest 45°
      const ux = Math.cos(ang);
      const uy = Math.sin(ang);
      const len = dx * ux + dy * uy; // project the cursor onto the snapped ray
      return h.coordAt(f.x + len * ux, f.y + len * uy) ?? free;
    };

    // Data coord for a moving box corner at pixel (x,y), opposite the `fixed`
    // corner. With Shift held, constrains the box to a perfect square (equal side
    // lengths in px), enclosing the cursor — the box analogue of snapToLine's 45°.
    const snapToSquare = (
      fixed: DrawPoint,
      x: number,
      y: number,
    ): { timeMs: number; price: number } | null => {
      const h = handleRef.current;
      if (!h) return null;
      const free = h.coordAt(x, y);
      if (!shiftHeld || !free) return free;
      const f = h.project(fixed.timeMs, fixed.price);
      if (!f) return free;
      const dx = x - f.x;
      const dy = y - f.y;
      const side = Math.max(Math.abs(dx), Math.abs(dy)); // equal side = larger delta
      if (side === 0) return free;
      const nx = f.x + (dx < 0 ? -side : side);
      const ny = f.y + (dy < 0 ? -side : side);
      return h.coordAt(nx, ny) ?? free;
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

    // True while a drawing tool should own input (suppress pan/zoom/crosshair).
    const drawActive = () =>
      optsRef.current.mode === 'draw' &&
      (optsRef.current.tool === 'line' || optsRef.current.tool === 'box');

    // Whether the active draw tool is the box (vs the line). `1`/`0` maps to the
    // core's draft/drawing `kind`.
    const drawIsBox = () => optsRef.current.tool === 'box';

    // Snap the moving anchor relative to `fixed`, using the constraint that fits
    // the shape: square for a box, 45° for a line.
    const snapMoving = (isBox: boolean, fixed: DrawPoint, x: number, y: number) =>
      isBox ? snapToSquare(fixed, x, y) : snapToLine(fixed, x, y);

    // While placing the second point, keep the live preview glued to the cursor.
    const updateGuideline = (x: number, y: number) => {
      const h = handleRef.current;
      if (!h) return;
      const a = drawAnchorRef.current;
      if (!a) return;
      const isBox = drawIsBox();
      const c = snapMoving(isBox, a, x, y);
      if (!c) return;
      h.setDraft(a.timeMs, a.price, true, c.timeMs, c.price, true, DRAW_COLOR, DRAW_WIDTH, isBox ? 1 : 0);
      scheduleRender();
    };

    // Draw-tool tap: place the first point, then commit on the second. Works for
    // both the line (two endpoints) and the box (two opposite corners).
    const handleDrawClick = (x: number, y: number) => {
      const h = handleRef.current;
      if (!h) return;

      const isBox = drawIsBox();
      if (!drawAnchorRef.current) {
        // First point: show node A; the preview follows on the next move.
        const coord = h.coordAt(x, y);
        if (!coord) return;
        drawAnchorRef.current = coord;
        h.setDraft(coord.timeMs, coord.price, false, 0, 0, true, DRAW_COLOR, DRAW_WIDTH, isBox ? 1 : 0);
        scheduleRender();
      } else {
        // Second point: commit the shape (Shift-snapped to match the preview)
        // and clear the draft. Editing happens in pan mode via the committed shape.
        const a = drawAnchorRef.current;
        const coord = snapMoving(isBox, a, x, y);
        if (!coord) return;
        optsRef.current.onDrawingComplete?.({
          id: drawingId(),
          type: isBox ? 'box' : 'line',
          points: [
            { timeMs: a.timeMs, price: a.price },
            { timeMs: coord.timeMs, price: coord.price },
          ],
        });
        drawAnchorRef.current = null;
        h.clearDraft();
        scheduleRender();
      }
    };

    // Reposition the grabbed endpoint at pixel (x,y), Shift-snapping to 45°
    // relative to the fixed (other) endpoint.
    const moveGrab = (x: number, y: number) => {
      const h = handleRef.current;
      const g = grabRef.current;
      if (!h || !g) return;
      const c = snapMoving(g.isBox, g.fixed, x, y);
      if (!c) return;
      lastCoordRef.current = c;
      h.moveDrawingEndpoint(g.index, g.endpoint, c.timeMs, c.price);
      scheduleRender();
    };

    const onPointerDown = (e: PointerEvent) => {
      const h = handleRef.current;
      if (!h) return;
      if (e.pointerType === 'mouse' && e.button !== 0) return; // left button only
      const { x, y } = rel(e);
      shiftHeld = e.shiftKey;
      lastX = x;
      lastY = y;
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

      // Editing (pan mode, chart area): grab a handle to drag it now; otherwise
      // record the hit so a stationary release can select a body / deselect.
      downHitRef.current = null;
      if (!drawActive() && panMode === 'chart') {
        const hit = h.hitTestDrawing(x, y);
        const gd = hit ? (optsRef.current.drawings ?? [])[hit.index] : undefined;
        const isBox = gd?.type === 'box';
        // Handle/corner grab: line endpoints (part 0/1) or box corners (part 0..3).
        const isHandle = hit != null && (isBox ? hit.part <= 3 : hit.part <= 1);
        if (hit && gd && isHandle) {
          if (isBox) {
            // A box corner: the diagonally opposite corner stays fixed. Normalize
            // the core so endpoint 0 = grabbed corner, endpoint 1 = diagonal, then
            // the line's endpoint-drag path resizes it (all corners stay 90°).
            const corners = boxCorners(gd.points);
            const grabbed = corners[hit.part];
            const diagonal = corners[(hit.part + 2) % 4];
            h.moveDrawingEndpoint(hit.index, 0, grabbed.timeMs, grabbed.price);
            h.moveDrawingEndpoint(hit.index, 1, diagonal.timeMs, diagonal.price);
            grabRef.current = { index: hit.index, endpoint: 0, fixed: diagonal, isBox: true };
            lastCoordRef.current = grabbed;
          } else {
            grabRef.current = {
              index: hit.index,
              endpoint: hit.part,
              fixed: gd.points[hit.part === 0 ? 1 : 0], // the other end stays put
              isBox: false,
            };
            lastCoordRef.current = null;
          }
          applySelection(); // enlarge the grabbed handle
          return; // the handle drag owns this pointer — no pan / long-press
        }
        // Body of the already-selected shape → translate the whole shape on drag.
        // Line body is part 2; box body (interior/edge) is part 4.
        const isBody = hit != null && (isBox ? hit.part === 4 : hit.part === 2);
        if (hit && isBody) {
          const list = optsRef.current.drawings ?? [];
          const selIdx = selectedIdRef.current
            ? list.findIndex((d) => d.id === selectedIdRef.current)
            : -1;
          if (hit.index === selIdx && selIdx >= 0) {
            const start = h.coordAt(x, y);
            const d = list[selIdx];
            if (start && d) {
              dragLineRef.current = {
                index: selIdx,
                startTime: start.timeMs,
                startPrice: start.price,
                a0: d.points[0],
                b0: d.points[1],
                lastA: d.points[0],
                lastB: d.points[1],
              };
              el.style.cursor = 'move';
              return; // the shape drag owns this pointer — no pan / long-press
            }
          }
        }
        downHitRef.current = hit; // non-selected body or miss → resolved on release
      }

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
      shiftHeld = e.shiftKey;
      lastX = x;
      lastY = y;

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

      // Handle drag (pan-mode editing): reposition the grabbed endpoint live,
      // with no move threshold, and never pan.
      if (grabRef.current) {
        moveGrab(x, y);
        return;
      }

      // Line translate (pan-mode editing): shift both endpoints by the pointer's
      // data-space delta, live, no threshold, never pan.
      if (dragLineRef.current) {
        const g = dragLineRef.current;
        const c = h.coordAt(x, y);
        if (c) {
          const dT = c.timeMs - g.startTime;
          const dP = c.price - g.startPrice;
          g.lastA = { timeMs: g.a0.timeMs + dT, price: g.a0.price + dP };
          g.lastB = { timeMs: g.b0.timeMs + dT, price: g.b0.price + dP };
          h.moveDrawingEndpoint(g.index, 0, g.lastA.timeMs, g.lastA.price);
          h.moveDrawingEndpoint(g.index, 1, g.lastB.timeMs, g.lastB.price);
          if (!moved && Math.hypot(x - downX, y - downY) > MOVE_THRESH) moved = true;
          scheduleRender();
        }
        return;
      }

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
      shiftHeld = e.shiftKey; // so a Shift-held draw commit matches the guideline
      const had = pointers.delete(e.pointerId);
      clearLongPress();
      if (pointers.size < 2) pinch.active = false;
      if (!had) return;

      // Handle-drag release (editing): persist the moved endpoint, keep it
      // selected, and un-grab (drops the 50% enlarge).
      if (grabRef.current && pointers.size === 0) {
        const g = grabRef.current;
        grabRef.current = null;
        const list = optsRef.current.drawings ?? [];
        const d = list[g.index];
        const c = lastCoordRef.current;
        if (d && c) {
          // Box: the two opposite corners are the dragged corner + its fixed
          // diagonal (order-independent). Line: replace the grabbed endpoint,
          // keeping the other in place.
          const pts: [DrawPoint, DrawPoint] = g.isBox
            ? [{ timeMs: c.timeMs, price: c.price }, g.fixed]
            : [d.points[0], d.points[1]];
          if (!g.isBox) pts[g.endpoint] = { timeMs: c.timeMs, price: c.price };
          optsRef.current.onDrawingChange?.({ ...d, points: pts });
        }
        applySelection();
        el.style.cursor = '';
        return;
      }

      // Line-translate release: persist the moved line, keep it selected.
      if (dragLineRef.current && pointers.size === 0) {
        const g = dragLineRef.current;
        dragLineRef.current = null;
        if (moved) {
          const list = optsRef.current.drawings ?? [];
          const d = list[g.index];
          if (d) optsRef.current.onDrawingChange?.({ ...d, points: [g.lastA, g.lastB] });
        }
        applySelection();
        el.style.cursor = '';
        return;
      }

      // Draw mode: a stationary tap in the chart places a point (or exits on
      // click-away). Drags are ignored (no pan). Crosshair logic is skipped.
      if (drawActive() && pointers.size === 0) {
        if (!moved && panMode === 'chart') handleDrawClick(downX, downY);
        else el.style.cursor = 'crosshair';
        return;
      }

      // Editing tap (pan mode): a stationary click on a line body selects it;
      // on empty space it deselects. (Skip a long-press, which owns the crosshair.)
      if (pointers.size === 0 && !moved && panMode === 'chart' && crosshairSource !== 'press') {
        const hit = downHitRef.current;
        const list = optsRef.current.drawings ?? [];
        const hitDrawing = hit ? list[hit.index] : undefined;
        // Body hit: line body is part 2, box body is part 4.
        const isBodyHit =
          hit != null &&
          hitDrawing != null &&
          (hitDrawing.type === 'box' ? hit.part === 4 : hit.part === 2);
        if (hit && isBodyHit && hitDrawing) {
          selectedTRef.current = hit.t; // remember where along the line it was grabbed
          if (selectedIdRef.current !== hitDrawing.id) {
            selectedIdRef.current = hitDrawing.id;
            applySelection();
          }
        } else if (!hit && selectedIdRef.current != null) {
          selectedIdRef.current = null;
          applySelection();
        }
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

    // Shift press/release re-applies the 45° constraint live at the last cursor
    // position — even with the mouse still — while drawing or dragging a handle.
    const onShiftKey = (e: KeyboardEvent) => {
      if (e.key !== 'Shift') return;
      const held = e.type === 'keydown';
      if (held === shiftHeld) return;
      shiftHeld = held;
      if (drawAnchorRef.current) updateGuideline(lastX, lastY);
      else if (grabRef.current) moveGrab(lastX, lastY);
    };

    el.addEventListener('pointerdown', onPointerDown);
    el.addEventListener('pointermove', onPointerMove);
    el.addEventListener('pointerup', endPointer);
    el.addEventListener('pointercancel', endPointer);
    el.addEventListener('pointerleave', onPointerLeave);
    el.addEventListener('wheel', onWheel, { passive: false });
    window.addEventListener('keydown', onShiftKey);
    window.addEventListener('keyup', onShiftKey);

    return () => {
      clearLongPress();
      el.removeEventListener('pointerdown', onPointerDown);
      el.removeEventListener('pointermove', onPointerMove);
      el.removeEventListener('pointerup', endPointer);
      el.removeEventListener('pointercancel', endPointer);
      el.removeEventListener('pointerleave', onPointerLeave);
      el.removeEventListener('wheel', onWheel);
      window.removeEventListener('keydown', onShiftKey);
      window.removeEventListener('keyup', onShiftKey);
    };
  }, [containerRef, handleRef, scheduleRender]);
}
