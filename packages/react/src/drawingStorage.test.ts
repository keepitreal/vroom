import { describe, expect, it } from 'vitest';
import type { Drawing } from '@vroomchart/types';

import { DRAWINGS_VERSION, deserializeDrawings, serializeDrawings } from './drawingStorage';

const p = (timeMs: number, price: number) => ({ timeMs, price });

const line: Drawing = { id: 'l1', type: 'line', points: [p(1000, 10), p(2000, 20)] };
const box: Drawing = { id: 'b1', type: 'box', points: [p(1000, 10), p(2000, 20)] };
const pencil: Drawing = {
  id: 'p1',
  type: 'pencil',
  points: [p(1000, 10), p(1100, 12), p(1200, 11), p(1300, 15)],
};

/** Wrap raw entries in a current-version envelope, bypassing serializeDrawings. */
const envelope = (drawings: unknown[]) =>
  JSON.stringify({ v: DRAWINGS_VERSION, drawings });

describe('serialize / deserialize round-trip', () => {
  it('round-trips a pencil stroke with all of its points intact', () => {
    const out = deserializeDrawings(serializeDrawings([pencil]));
    expect(out).toEqual([pencil]);
    expect(out[0]!.points).toHaveLength(4);
  });

  it('round-trips a mixed set of line, box and pencil', () => {
    const all = [line, box, pencil];
    expect(deserializeDrawings(serializeDrawings(all))).toEqual(all);
  });

  it('preserves optional color and width on a stroke', () => {
    const styled: Drawing = { ...pencil, color: '#ff0000', width: 4 };
    expect(deserializeDrawings(serializeDrawings([styled]))).toEqual([styled]);
  });

  it('keeps point order (a stroke is directional)', () => {
    const out = deserializeDrawings(serializeDrawings([pencil]));
    expect(out[0]!.points.map((q) => q.timeMs)).toEqual([1000, 1100, 1200, 1300]);
  });
});

describe('malformed entries are dropped, not rendered', () => {
  it('drops a pencil whose points is not an array', () => {
    expect(deserializeDrawings(envelope([{ id: 'x', type: 'pencil', points: 'nope' }]))).toEqual([]);
  });

  it('drops a pencil with fewer than 2 points', () => {
    expect(deserializeDrawings(envelope([{ id: 'x', type: 'pencil', points: [p(1, 1)] }]))).toEqual([]);
  });

  it('drops a stroke containing a NaN coordinate', () => {
    const bad = { id: 'x', type: 'pencil', points: [p(1, 1), p(NaN, 2), p(3, 3)] };
    expect(deserializeDrawings(envelope([bad]))).toEqual([]);
  });

  it('drops a stroke containing a malformed point object', () => {
    const bad = { id: 'x', type: 'pencil', points: [p(1, 1), { timeMs: 2 }] };
    expect(deserializeDrawings(envelope([bad]))).toEqual([]);
  });

  it('drops a line that does not have exactly 2 points', () => {
    const three = { id: 'x', type: 'line', points: [p(1, 1), p(2, 2), p(3, 3)] };
    expect(deserializeDrawings(envelope([three]))).toEqual([]);
  });

  it('still drops an unknown drawing type', () => {
    const future = { id: 'x', type: 'ellipse', points: [p(1, 1), p(2, 2)] };
    expect(deserializeDrawings(envelope([future]))).toEqual([]);
  });

  it('keeps the valid entries alongside corrupt ones', () => {
    const mixed = envelope([pencil, { id: 'x', type: 'pencil', points: null }, line]);
    expect(deserializeDrawings(mixed)).toEqual([pencil, line]);
  });

  it('never throws on garbage input', () => {
    expect(deserializeDrawings('not json')).toEqual([]);
    expect(deserializeDrawings('')).toEqual([]);
    expect(deserializeDrawings(null)).toEqual([]);
    expect(deserializeDrawings(undefined)).toEqual([]);
    expect(deserializeDrawings('{"v":1}')).toEqual([]);
  });
});

describe('backward compatibility', () => {
  it('loads a line/box payload written before the pencil tool existed', () => {
    const legacy = JSON.stringify({ v: 1, drawings: [line, box] });
    expect(deserializeDrawings(legacy)).toEqual([line, box]);
  });

  it('loads a bare array as the pre-envelope v0 shape', () => {
    expect(deserializeDrawings(JSON.stringify([line]))).toEqual([line]);
  });
});
