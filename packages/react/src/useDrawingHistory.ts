// Controlled-mode counterpart to the managed history in useManagedDrawings:
// owns a Drawing[] plus its undo/redo stack, and hands back exactly the props
// the chart needs. Managed mode (`drawingStore`) has this built in — reach for
// this hook when you own the drawings array yourself:
//
//   const h = useDrawingHistory();
//   <VroomChart
//     drawings={h.drawings}
//     onDrawingComplete={h.onDrawingComplete}
//     onDrawingChange={h.onDrawingChange}
//     onDrawingDelete={h.onDrawingDelete}
//     onUndo={h.undo}
//     onRedo={h.redo}
//   />
//
// Persist `drawings` however you like (it's plain state); the history itself is
// in-memory and session-only on purpose — see drawingHistory.ts.

import { useCallback, useRef, useState } from 'react';
import type { Drawing, UndoRedoState } from '@vroomchart/types';

import { DrawingHistory, DEFAULT_HISTORY_LIMIT } from './drawingHistory';

export type DrawingHistoryApi = {
  /** The drawings to pass to the chart's `drawings` prop. */
  drawings: Drawing[];
  /** Pass to the chart prop of the same name. */
  onDrawingComplete: (d: Drawing) => void;
  /** Pass to the chart prop of the same name. */
  onDrawingChange: (d: Drawing) => void;
  /** Pass to the chart prop of the same name. */
  onDrawingDelete: (id: string) => void;
  /** Roll back the last committed action. Wire to the chart's `onUndo` and/or a button. */
  undo: () => void;
  /** Re-apply the last undone action. Wire to the chart's `onRedo` and/or a button. */
  redo: () => void;
  canUndo: boolean;
  canRedo: boolean;
  /**
   * Replace the whole array as a document operation — loading saved drawings,
   * switching markets — without recording an undo step. Also drops the existing
   * history, which was recorded against the outgoing document.
   */
  replaceAll: (drawings: Drawing[]) => void;
  /** Drop the undo/redo stacks without touching the drawings. */
  clearHistory: () => void;
};

/**
 * Owns a drawings array plus a bounded undo/redo history (one step per
 * committed drawing action; default depth {@link DEFAULT_HISTORY_LIMIT}).
 */
export function useDrawingHistory(
  options: { limit?: number; initial?: Drawing[] } = {},
): DrawingHistoryApi {
  const [drawings, setDrawings] = useState<Drawing[]>(options.initial ?? []);
  const [state, setState] = useState<UndoRedoState>({ canUndo: false, canRedo: false });

  const drawingsRef = useRef(drawings);
  const historyRef = useRef<DrawingHistory | null>(null);
  if (historyRef.current == null) {
    historyRef.current = new DrawingHistory(options.limit ?? DEFAULT_HISTORY_LIMIT);
  }
  if (options.limit != null) historyRef.current.limit = Math.max(1, options.limit);

  const setBoth = (list: Drawing[]) => {
    drawingsRef.current = list;
    setDrawings(list);
  };
  const syncState = () => {
    const h = historyRef.current!;
    setState((p) => (p.canUndo === h.canUndo && p.canRedo === h.canRedo ? p : h.state));
  };

  const onDrawingComplete = useCallback((d: Drawing) => {
    historyRef.current!.recordAdd(d);
    setBoth([...drawingsRef.current, d]);
    syncState();
  }, []);
  const onDrawingChange = useCallback((d: Drawing) => {
    const before = drawingsRef.current.find((x) => x.id === d.id);
    if (before) historyRef.current!.recordChange(before, d);
    setBoth(drawingsRef.current.map((x) => (x.id === d.id ? d : x)));
    syncState();
  }, []);
  const onDrawingDelete = useCallback((id: string) => {
    const index = drawingsRef.current.findIndex((x) => x.id === id);
    if (index >= 0) historyRef.current!.recordDelete(drawingsRef.current[index]!, index);
    setBoth(drawingsRef.current.filter((x) => x.id !== id));
    syncState();
  }, []);

  const undo = useCallback(() => {
    const next = historyRef.current!.undo(drawingsRef.current);
    if (next != null) setBoth(next);
    syncState();
  }, []);
  const redo = useCallback(() => {
    const next = historyRef.current!.redo(drawingsRef.current);
    if (next != null) setBoth(next);
    syncState();
  }, []);

  const replaceAll = useCallback((list: Drawing[]) => {
    historyRef.current!.clear();
    setBoth(list);
    syncState();
  }, []);
  const clearHistory = useCallback(() => {
    historyRef.current!.clear();
    syncState();
  }, []);

  return {
    drawings,
    onDrawingComplete,
    onDrawingChange,
    onDrawingDelete,
    undo,
    redo,
    canUndo: state.canUndo,
    canRedo: state.canRedo,
    replaceAll,
    clearHistory,
  };
}
