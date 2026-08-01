import { describe, expect, it } from 'vitest';
import type { Drawing } from '@vroomchart/types';

import { DrawingHistory } from './drawingHistory';

const p = (timeMs: number, price: number) => ({ timeMs, price });

const strokeA: Drawing = {
  id: 'a',
  type: 'pencil',
  points: [p(1000, 10), p(1100, 12), p(1200, 11)],
};
const strokeB: Drawing = {
  id: 'b',
  type: 'pencil',
  points: [p(2000, 20), p(2100, 22)],
};
const box: Drawing = { id: 'x', type: 'box', points: [p(1000, 10), p(2000, 20)] };

/** Drive the history the way the mutation handlers do: record + apply. */
const add = (h: DrawingHistory, list: Drawing[], d: Drawing): Drawing[] => {
  h.recordAdd(d);
  return [...list, d];
};

describe('two committed pencil strokes (the canonical scenario)', () => {
  it('undo removes only the last stroke, then the first; redo restores in order', () => {
    const h = new DrawingHistory();
    let list: Drawing[] = [];
    list = add(h, list, strokeA);
    list = add(h, list, strokeB);

    list = h.undo(list)!;
    expect(list).toEqual([strokeA]); // only the last stroke removed

    list = h.undo(list)!;
    expect(list).toEqual([]);
    expect(h.canUndo).toBe(false);

    list = h.redo(list)!;
    expect(list).toEqual([strokeA]); // first stroke back first

    list = h.redo(list)!;
    expect(list).toEqual([strokeA, strokeB]);
    expect(h.canRedo).toBe(false);
  });
});

describe('inverse operations', () => {
  it('undoes a change by restoring the before-drawing, and redoes the after', () => {
    const h = new DrawingHistory();
    const moved: Drawing = { ...strokeA, points: [p(1000, 15), p(1100, 17), p(1200, 16)] };
    let list: Drawing[] = [strokeA, strokeB];
    h.recordChange(strokeA, moved);
    list = list.map((d) => (d.id === moved.id ? moved : d));

    list = h.undo(list)!;
    expect(list).toEqual([strokeA, strokeB]);

    list = h.redo(list)!;
    expect(list).toEqual([moved, strokeB]);
  });

  it('undoes a delete by re-inserting at the original index (z-order preserved)', () => {
    const h = new DrawingHistory();
    let list: Drawing[] = [strokeA, box, strokeB];
    h.recordDelete(box, 1);
    list = list.filter((d) => d.id !== box.id);

    list = h.undo(list)!;
    expect(list.map((d) => d.id)).toEqual(['a', 'x', 'b']);
  });

  it('clamps the delete re-insert index when the list has since shrunk', () => {
    const h = new DrawingHistory();
    h.recordDelete(box, 5);
    expect(h.undo([strokeA])!.map((d) => d.id)).toEqual(['a', 'x']);
  });

  it('ignores a change record whose before/after ids disagree', () => {
    const h = new DrawingHistory();
    h.recordChange(strokeA, strokeB);
    expect(h.canUndo).toBe(false);
  });
});

describe('linear history', () => {
  it('clears the redo stack when a new action is recorded after an undo', () => {
    const h = new DrawingHistory();
    let list: Drawing[] = [];
    list = add(h, list, strokeA);
    list = h.undo(list)!;
    expect(h.canRedo).toBe(true);

    list = add(h, list, strokeB); // new branch — the undone stroke is gone for good
    expect(h.canRedo).toBe(false);
    expect(h.redo(list)).toBeNull();
    expect(list).toEqual([strokeB]);
  });

  it('returns null on undo/redo with empty stacks', () => {
    const h = new DrawingHistory();
    expect(h.undo([strokeA])).toBeNull();
    expect(h.redo([strokeA])).toBeNull();
  });
});

describe('depth limit', () => {
  it('evicts the oldest entries beyond the limit', () => {
    const h = new DrawingHistory(2);
    let list: Drawing[] = [];
    list = add(h, list, strokeA);
    list = add(h, list, strokeB);
    list = add(h, list, box); // strokeA's entry evicted

    list = h.undo(list)!;
    list = h.undo(list)!;
    expect(h.canUndo).toBe(false); // only 2 steps back
    expect(list).toEqual([strokeA]); // strokeA itself survives — only its history entry is gone
  });

  it('applies a lowered limit on the next record', () => {
    const h = new DrawingHistory(10);
    let list: Drawing[] = [];
    list = add(h, list, strokeA);
    list = add(h, list, strokeB);
    h.limit = 1;
    list = add(h, list, box);
    list = h.undo(list)!;
    expect(h.canUndo).toBe(false);
    expect(list).toEqual([strokeA, strokeB]);
  });
});

describe('clear', () => {
  it('drops both stacks', () => {
    const h = new DrawingHistory();
    let list: Drawing[] = [];
    list = add(h, list, strokeA);
    list = h.undo(list)!;
    h.clear();
    expect(h.state).toEqual({ canUndo: false, canRedo: false });
  });
});
