// Managed drawing persistence. When a `drawingStore` is provided, the chart owns
// the drawings array itself and loads/saves it through the store, keyed by
// `seriesKey` (the market). Drawings are data-space anchored, so keying by market
// means they persist across timeframe changes but not across markets.
//
// Also owns the drawings' undo/redo history: one step per committed action,
// recorded in the same handlers that mutate the array. The stack is in-memory
// and cleared on market switch — the drawings persist, the history doesn't
// (see drawingHistory.ts for the rationale).
//
// Returns the internal drawings plus the three edit handlers to feed into the
// gesture layer (in place of the controlled props). A no-op when `store` is
// undefined, so it can be called unconditionally (rules of hooks).

import { useCallback, useEffect, useRef, useState } from 'react';
import type { Drawing, DrawingStore, DrawingStyle, UndoRedoState } from '@vroomchart/types';

import { serializeDrawings, deserializeDrawings } from './drawingStorage';
import { DrawingHistory, DEFAULT_HISTORY_LIMIT } from './drawingHistory';

const SAVE_DEBOUNCE_MS = 400;

export type ManagedDrawings = {
  drawings: Drawing[];
  onDrawingComplete: (d: Drawing) => void;
  onDrawingChange: (d: Drawing) => void;
  onDrawingDelete: (id: string) => void;
  restyle: (id: string, patch: DrawingStyle) => void;
  undo: () => void;
  redo: () => void;
  clearHistory: () => void;
  historyState: UndoRedoState;
};

const HISTORY_EMPTY: UndoRedoState = { canUndo: false, canRedo: false };

/**
 * The before/after pair for restyling the drawing with `id`, or null when the
 * list has no such drawing. Split out from the hook so the merge rules are
 * testable without a renderer.
 *
 * The patch is merged shallowly and `before` is left untouched, which is what
 * lets the pair go straight into the undo history as one step.
 */
export function restyleChange(
  list: Drawing[],
  id: string,
  patch: DrawingStyle,
): { before: Drawing; after: Drawing } | null {
  const before = list.find((d) => d.id === id);
  if (!before) return null;
  return { before, after: { ...before, ...patch } as Drawing };
}

export function useManagedDrawings(
  seriesKey: string | undefined,
  store: DrawingStore | undefined,
  historyLimit: number = DEFAULT_HISTORY_LIMIT,
): ManagedDrawings {
  const [drawings, setDrawings] = useState<Drawing[]>([]);
  const [historyState, setHistoryState] = useState<UndoRedoState>(HISTORY_EMPTY);

  // Latest-committed refs so the load effect / debounce read current values
  // without re-subscribing.
  const drawingsRef = useRef<Drawing[]>([]);
  const storeRef = useRef(store);
  storeRef.current = store;
  // The market whose drawings are currently in state — so we never save one
  // market's drawings under another's key during an async load race.
  const loadedKeyRef = useRef<string | undefined>(undefined);
  const loadTokenRef = useRef(0);
  const saveTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const warnedRef = useRef(false);

  const historyRef = useRef<DrawingHistory | null>(null);
  if (historyRef.current == null) historyRef.current = new DrawingHistory(historyLimit);
  historyRef.current.limit = Math.max(1, historyLimit);

  const setBoth = (list: Drawing[]) => {
    drawingsRef.current = list;
    setDrawings(list);
  };

  // Re-render only when availability actually flips (the stacks live in a ref).
  const syncHistoryState = () => {
    const h = historyRef.current!;
    setHistoryState((p) =>
      p.canUndo === h.canUndo && p.canRedo === h.canRedo ? p : h.state,
    );
  };

  const resetHistory = () => {
    historyRef.current!.clear();
    syncHistoryState();
  };

  // Persist immediately (cancelling any pending debounce). Best-effort.
  const flushSave = useCallback((key: string | undefined, list: Drawing[]) => {
    if (saveTimerRef.current != null) {
      clearTimeout(saveTimerRef.current);
      saveTimerRef.current = null;
    }
    const s = storeRef.current;
    if (!s || key == null) return;
    try {
      // The store is an opaque string KV — the library owns the versioned
      // envelope, so serialize here and hand over bytes.
      const r = s.save(key, serializeDrawings(list));
      if (r && typeof (r as Promise<void>).catch === 'function') {
        (r as Promise<void>).catch(() => {});
      }
    } catch {
      /* best-effort */
    }
  }, []);

  const scheduleSave = useCallback(() => {
    const key = seriesKey;
    if (!storeRef.current || key == null) return;
    if (saveTimerRef.current != null) clearTimeout(saveTimerRef.current);
    saveTimerRef.current = setTimeout(() => {
      saveTimerRef.current = null;
      flushSave(key, drawingsRef.current);
    }, SAVE_DEBOUNCE_MS);
  }, [seriesKey, flushSave]);

  // Load on mount / market change; flush-save the outgoing market on the way out.
  // Loading is a document replacement, not a user action: it never records
  // history, and any history from the previous market is dropped so undo can't
  // cross-apply to the wrong document.
  useEffect(() => {
    const s = storeRef.current;
    if (!s) return;
    resetHistory();
    if (seriesKey == null) {
      if (!warnedRef.current && typeof console !== 'undefined') {
        console.warn(
          '[vroom] drawingStore is set but seriesKey is missing — drawings will render but not persist.',
        );
        warnedRef.current = true;
      }
      setBoth([]);
      loadedKeyRef.current = undefined;
      return;
    }

    const token = ++loadTokenRef.current;
    const result = s.load(seriesKey);
    if (result && typeof (result as Promise<string | null>).then === 'function') {
      // Async: clear now so stale drawings don't show, apply when resolved.
      setBoth([]);
      loadedKeyRef.current = undefined;
      (result as Promise<string | null | undefined>)
        .then((raw) => {
          if (token !== loadTokenRef.current) return; // superseded by a newer switch
          loadedKeyRef.current = seriesKey;
          setBoth(deserializeDrawings(raw));
        })
        .catch(() => {
          if (token !== loadTokenRef.current) return;
          loadedKeyRef.current = seriesKey;
          setBoth([]);
        });
    } else {
      loadedKeyRef.current = seriesKey;
      setBoth(deserializeDrawings(result as string | null | undefined));
    }

    // On the next market switch (or unmount), save this market's drawings — but
    // only if the in-memory set actually belongs to this key (async-load guard).
    return () => {
      if (loadedKeyRef.current === seriesKey) {
        flushSave(seriesKey, drawingsRef.current);
      } else if (saveTimerRef.current != null) {
        clearTimeout(saveTimerRef.current);
        saveTimerRef.current = null;
      }
    };
  }, [seriesKey, flushSave]);

  const onDrawingComplete = useCallback(
    (d: Drawing) => {
      historyRef.current!.recordAdd(d);
      syncHistoryState();
      setDrawings((p) => {
        const next = [...p, d];
        drawingsRef.current = next;
        return next;
      });
      scheduleSave();
    },
    [scheduleSave],
  );
  const onDrawingChange = useCallback(
    (d: Drawing) => {
      const before = drawingsRef.current.find((x) => x.id === d.id);
      if (before) {
        historyRef.current!.recordChange(before, d);
        syncHistoryState();
      }
      setDrawings((p) => {
        const next = p.map((x) => (x.id === d.id ? d : x));
        drawingsRef.current = next;
        return next;
      });
      scheduleSave();
    },
    [scheduleSave],
  );
  const onDrawingDelete = useCallback(
    (id: string) => {
      const index = drawingsRef.current.findIndex((x) => x.id === id);
      if (index >= 0) {
        historyRef.current!.recordDelete(drawingsRef.current[index]!, index);
        syncHistoryState();
      }
      setDrawings((p) => {
        const next = p.filter((x) => x.id !== id);
        drawingsRef.current = next;
        return next;
      });
      scheduleSave();
    },
    [scheduleSave],
  );

  // The host's way in. Gestures aside, this is the only thing that can change a
  // managed drawing — the array lives in here, so "make this one red" is
  // otherwise unsayable. Goes through the same recordChange as a drag, so a
  // restyle is one undo step like every other edit.
  const restyle = useCallback(
    (id: string, patch: DrawingStyle) => {
      const change = restyleChange(drawingsRef.current, id, patch);
      if (!change) return;
      historyRef.current!.recordChange(change.before, change.after);
      syncHistoryState();
      setBoth(drawingsRef.current.map((x) => (x.id === id ? change.after : x)));
      scheduleSave();
    },
    [scheduleSave],
  );

  const undo = useCallback(() => {
    const next = historyRef.current!.undo(drawingsRef.current);
    syncHistoryState();
    if (next == null) return;
    setBoth(next);
    scheduleSave(); // the persisted document tracks the undone state
  }, [scheduleSave]);
  const redo = useCallback(() => {
    const next = historyRef.current!.redo(drawingsRef.current);
    syncHistoryState();
    if (next == null) return;
    setBoth(next);
    scheduleSave();
  }, [scheduleSave]);
  const clearHistory = useCallback(() => {
    historyRef.current!.clear();
    setHistoryState((p) => (p.canUndo || p.canRedo ? { canUndo: false, canRedo: false } : p));
  }, []);

  return {
    drawings,
    onDrawingComplete,
    onDrawingChange,
    onDrawingDelete,
    restyle,
    undo,
    redo,
    clearHistory,
    historyState,
  };
}
