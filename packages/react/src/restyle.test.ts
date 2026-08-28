import { describe, expect, it } from 'vitest';
import type { Drawing } from '@vroomchart/types';

import { DrawingHistory } from './drawingHistory';
import { restyleChange } from './useManagedDrawings';

const p = (timeMs: number, price: number) => ({ timeMs, price });

const line: Drawing = { id: 'l1', type: 'line', points: [p(1000, 10), p(2000, 20)] };
const box: Drawing = {
  id: 'b1',
  type: 'box',
  points: [p(1000, 10), p(2000, 20)],
  color: '#ff2962ff',
};
const list: Drawing[] = [line, box];

describe('restyleChange', () => {
  it('merges the patch over the drawing it names', () => {
    const change = restyleChange(list, 'b1', { color: '#ffff0000', width: 4 });
    expect(change?.after).toEqual({ ...box, color: '#ffff0000', width: 4 });
  });

  it('leaves fields the patch omits alone', () => {
    const change = restyleChange(list, 'b1', { width: 6 });
    expect(change?.after).toMatchObject({ color: '#ff2962ff', width: 6 });
  });

  it('carries a box fill and the locked flag', () => {
    const change = restyleChange(list, 'b1', { fill: '#5400ce2c', locked: true });
    expect(change?.after).toMatchObject({ fill: '#5400ce2c', locked: true });
  });

  it('never touches geometry', () => {
    const change = restyleChange(list, 'l1', { color: '#ff00ff00' });
    expect(change?.after.points).toEqual(line.points);
  });

  it('does not mutate the original drawing', () => {
    restyleChange(list, 'b1', { color: '#ffff0000' });
    expect(box.color).toBe('#ff2962ff');
  });

  it('returns null for an unknown id, so the caller records nothing', () => {
    expect(restyleChange(list, 'nope', { color: '#ffff0000' })).toBeNull();
  });

  it('returns null against an empty list', () => {
    expect(restyleChange([], 'l1', { locked: true })).toBeNull();
  });
});

describe('a restyle is one undo step', () => {
  it('undo puts the original styling back', () => {
    const h = new DrawingHistory();
    const change = restyleChange(list, 'b1', { color: '#ffff0000' })!;
    h.recordChange(change.before, change.after);
    const applied = list.map((d) => (d.id === 'b1' ? change.after : d));

    expect(h.canUndo).toBe(true);
    expect(h.undo(applied)).toEqual(list);
  });

  it('redo re-applies it', () => {
    const h = new DrawingHistory();
    const change = restyleChange(list, 'b1', { locked: true })!;
    h.recordChange(change.before, change.after);
    const applied = list.map((d) => (d.id === 'b1' ? change.after : d));

    const undone = h.undo(applied)!;
    expect(h.redo(undone)).toEqual(applied);
  });
});
