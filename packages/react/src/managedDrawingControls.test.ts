// @vitest-environment jsdom
import { act, renderHook } from '@testing-library/react';
import { describe, expect, it } from 'vitest';
import type { Drawing, DrawingStore } from '@vroomchart/types';

import { useManagedDrawings } from './useManagedDrawings';

const p = (timeMs: number, price: number) => ({ timeMs, price });
const line: Drawing = {
  id: 'l1',
  type: 'line',
  points: [p(1000, 10), p(2000, 20)],
  meta: { source: 'agent' },
};

const memoryStore = (): DrawingStore => {
  const data = new Map<string, string>();
  return { load: (k) => data.get(k) ?? null, save: (k, v) => void data.set(k, v) };
};

const setup = () => renderHook(() => useManagedDrawings('BTC', memoryStore()));

describe('programmatic drawing controls', () => {
  it('add appends a drawing and records an undoable step', () => {
    const { result } = setup();
    let ok = false;
    act(() => {
      ok = result.current.add(line);
    });
    expect(ok).toBe(true);
    expect(result.current.drawings).toEqual([line]);
    expect(result.current.list()).toEqual([line]);
    expect(result.current.historyState.canUndo).toBe(true);

    act(() => result.current.undo());
    expect(result.current.drawings).toEqual([]);
  });

  it('add rejects a duplicate id or a malformed drawing', () => {
    const { result } = setup();
    act(() => void result.current.add(line));
    let dup = true;
    let bad = true;
    act(() => {
      dup = result.current.add({ ...line, points: [p(1, 1), p(2, 2)] });
      bad = result.current.add({ id: 'x', type: 'line', points: [p(1, 1)] } as unknown as Drawing);
    });
    expect(dup).toBe(false);
    expect(bad).toBe(false);
    expect(result.current.drawings).toEqual([line]);
  });

  it('update replaces by id and undoes back to the original', () => {
    const { result } = setup();
    act(() => void result.current.add(line));
    const moved: Drawing = { ...line, points: [p(1000, 30), p(2000, 40)] };
    let ok = false;
    act(() => {
      ok = result.current.update(moved);
    });
    expect(ok).toBe(true);
    expect(result.current.drawings).toEqual([moved]);

    act(() => result.current.undo());
    expect(result.current.drawings).toEqual([line]);
  });

  it('update and remove return false for an unknown id', () => {
    const { result } = setup();
    let updated = true;
    let removed = true;
    act(() => {
      updated = result.current.update({ ...line, id: 'nope' });
      removed = result.current.remove('nope');
    });
    expect(updated).toBe(false);
    expect(removed).toBe(false);
    expect(result.current.historyState.canUndo).toBe(false);
  });

  it('remove deletes by id and undo restores it', () => {
    const { result } = setup();
    act(() => void result.current.add(line));
    act(() => void result.current.remove('l1'));
    expect(result.current.drawings).toEqual([]);

    act(() => result.current.undo());
    expect(result.current.drawings).toEqual([line]);
  });
});
