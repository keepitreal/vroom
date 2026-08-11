// Undo/redo history for committed drawings. One entry per committed user action
// (a finished stroke/shape, an endpoint drag, a delete) — never per input event,
// so undoing after two pencil strokes removes only the last stroke. Entries are
// inverse operations (Excalidraw-style before/after partials) rather than full
// snapshots: each knows how to roll itself back and forward against the current
// drawings array, keyed by the stable Drawing.id.
//
// The stack is in-memory and session-only by design. The document (the drawings
// themselves) is what persists — via `drawingStore` or the consumer's own state —
// matching the document-persisted / history-ephemeral split every major drawing
// app uses (Excalidraw, tldraw, Figma, Photoshop).

import type { Drawing, UndoRedoState } from '@vroomchart/types';

/** One committed action. `delete` keeps the removal index so undo re-inserts at
 * the original z-order (drawings render in array order). */
export type DrawingHistoryEntry =
  | { kind: 'add'; drawing: Drawing }
  | { kind: 'change'; before: Drawing; after: Drawing }
  | { kind: 'delete'; drawing: Drawing; index: number };

export const DEFAULT_HISTORY_LIMIT = 100;

/** Roll `entry` back against `list` (the state after the entry was applied). */
function applyInverse(list: Drawing[], entry: DrawingHistoryEntry): Drawing[] {
  switch (entry.kind) {
    case 'add':
      return list.filter((d) => d.id !== entry.drawing.id);
    case 'change':
      return list.map((d) => (d.id === entry.after.id ? entry.before : d));
    case 'delete': {
      const next = list.slice();
      next.splice(Math.min(entry.index, next.length), 0, entry.drawing);
      return next;
    }
  }
}

/** Re-apply `entry` against `list` (the state before the entry was applied). */
function applyForward(list: Drawing[], entry: DrawingHistoryEntry): Drawing[] {
  switch (entry.kind) {
    case 'add':
      return [...list, entry.drawing];
    case 'change':
      return list.map((d) => (d.id === entry.before.id ? entry.after : d));
    case 'delete':
      return list.filter((d) => d.id !== entry.drawing.id);
  }
}

/**
 * Linear undo/redo stack over a `Drawing[]`. Pure and framework-free: `undo`/
 * `redo` take the current array and return the next one (or `null` when the
 * stack is empty), leaving state ownership to the caller. Recording a new entry
 * clears the redo stack (linear history, no branches); the undo stack is capped
 * at `limit` entries, evicting oldest.
 */
export class DrawingHistory {
  private undoStack: DrawingHistoryEntry[] = [];
  private redoStack: DrawingHistoryEntry[] = [];
  /** Max undo depth. May be lowered at runtime; excess entries evict on the next record. */
  limit: number;

  constructor(limit: number = DEFAULT_HISTORY_LIMIT) {
    this.limit = Math.max(1, limit);
  }

  get canUndo(): boolean {
    return this.undoStack.length > 0;
  }

  get canRedo(): boolean {
    return this.redoStack.length > 0;
  }

  get state(): UndoRedoState {
    return { canUndo: this.canUndo, canRedo: this.canRedo };
  }

  /** Record a committed action. Call at the same choke point that mutates the
   * drawings array (the onDrawingComplete/Change/Delete handlers) — never for
   * loads or other non-user replacements. */
  record(entry: DrawingHistoryEntry): void {
    this.undoStack.push(entry);
    this.redoStack.length = 0;
    const over = this.undoStack.length - Math.max(1, this.limit);
    if (over > 0) this.undoStack.splice(0, over);
  }

  recordAdd(drawing: Drawing): void {
    this.record({ kind: 'add', drawing });
  }

  /** No-op when `before`/`after` disagree on id (nothing coherent to invert). */
  recordChange(before: Drawing, after: Drawing): void {
    if (before.id !== after.id) return;
    this.record({ kind: 'change', before, after });
  }

  recordDelete(drawing: Drawing, index: number): void {
    this.record({ kind: 'delete', drawing, index });
  }

  /** Roll back the most recent entry. Returns the new array, or `null` when
   * there is nothing to undo (caller keeps `current` as-is). */
  undo(current: Drawing[]): Drawing[] | null {
    const entry = this.undoStack.pop();
    if (!entry) return null;
    this.redoStack.push(entry);
    return applyInverse(current, entry);
  }

  /** Re-apply the most recently undone entry. Returns the new array, or `null`
   * when there is nothing to redo. */
  redo(current: Drawing[]): Drawing[] | null {
    const entry = this.redoStack.pop();
    if (!entry) return null;
    this.undoStack.push(entry);
    return applyForward(current, entry);
  }

  /** Drop both stacks — e.g. when the chart switches to another market's
   * drawings, so undo can never mutate a document it wasn't recorded against. */
  clear(): void {
    this.undoStack.length = 0;
    this.redoStack.length = 0;
  }
}
