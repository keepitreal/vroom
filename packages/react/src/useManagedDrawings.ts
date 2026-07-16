// Managed drawing persistence. When a `drawingStore` is provided, the chart owns
// the drawings array itself and loads/saves it through the store, keyed by
// `seriesKey` (the market). Drawings are data-space anchored, so keying by market
// means they persist across timeframe changes but not across markets.
//
// Returns the internal drawings plus the three edit handlers to feed into the
// gesture layer (in place of the controlled props). A no-op when `store` is
// undefined, so it can be called unconditionally (rules of hooks).

import { useCallback, useEffect, useRef, useState } from 'react';
import type { Drawing, DrawingStore } from '@vroomchart/types';

import { serializeDrawings, deserializeDrawings } from './drawingStorage';

const SAVE_DEBOUNCE_MS = 400;

export type ManagedDrawings = {
  drawings: Drawing[];
  onDrawingComplete: (d: Drawing) => void;
  onDrawingChange: (d: Drawing) => void;
  onDrawingDelete: (id: string) => void;
};

export function useManagedDrawings(
  seriesKey: string | undefined,
  store: DrawingStore | undefined,
): ManagedDrawings {
  const [drawings, setDrawings] = useState<Drawing[]>([]);

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

  const setBoth = (list: Drawing[]) => {
    drawingsRef.current = list;
    setDrawings(list);
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
  useEffect(() => {
    const s = storeRef.current;
    if (!s) return;
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
      setDrawings((p) => {
        const next = p.filter((x) => x.id !== id);
        drawingsRef.current = next;
        return next;
      });
      scheduleSave();
    },
    [scheduleSave],
  );

  return { drawings, onDrawingComplete, onDrawingChange, onDrawingDelete };
}
