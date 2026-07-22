import { describe, expect, it } from 'vitest';

import { simplify, simplifyIndices, type Pt } from './simplify';

const pt = (x: number, y: number): Pt => ({ x, y });

describe('simplifyIndices', () => {
  it('keeps everything when there are 2 or fewer points', () => {
    expect(simplifyIndices([], 1)).toEqual([]);
    expect(simplifyIndices([pt(0, 0)], 1)).toEqual([0]);
    expect(simplifyIndices([pt(0, 0), pt(5, 5)], 1)).toEqual([0, 1]);
  });

  it('collapses a straight run to just its endpoints', () => {
    const line = [pt(0, 0), pt(1, 0), pt(2, 0), pt(3, 0), pt(4, 0), pt(5, 0)];
    expect(simplifyIndices(line, 1)).toEqual([0, 5]);
  });

  it('collapses a straight diagonal run to its endpoints', () => {
    const diag = [pt(0, 0), pt(1, 1), pt(2, 2), pt(3, 3), pt(4, 4)];
    expect(simplifyIndices(diag, 0.5)).toEqual([0, 4]);
  });

  it('preserves a spike that exceeds epsilon', () => {
    // A flat run with one point pushed well off the chord.
    const spike = [pt(0, 0), pt(1, 0), pt(2, 10), pt(3, 0), pt(4, 0)];
    expect(simplifyIndices(spike, 1)).toEqual([0, 2, 4]);
  });

  it('drops a deviation smaller than epsilon', () => {
    // Same shape, but the bump is only 0.5px off — below a 1px tolerance.
    const nudge = [pt(0, 0), pt(1, 0), pt(2, 0.5), pt(3, 0), pt(4, 0)];
    expect(simplifyIndices(nudge, 1)).toEqual([0, 4]);
  });

  it('is a no-op when epsilon is 0', () => {
    const wiggly = [pt(0, 0), pt(1, 0.1), pt(2, 0), pt(3, 0.2), pt(4, 0)];
    expect(simplifyIndices(wiggly, 0)).toEqual([0, 1, 2, 3, 4]);
  });

  it('always keeps the first and last point', () => {
    const curve = Array.from({ length: 50 }, (_, i) => pt(i, Math.sin(i / 5) * 20));
    const idx = simplifyIndices(curve, 1);
    expect(idx[0]).toBe(0);
    expect(idx[idx.length - 1]).toBe(49);
  });

  it('substantially thins a dense hand-drawn-ish stroke', () => {
    // 400 samples along a smooth curve, as a real drag would produce.
    const dense = Array.from({ length: 400 }, (_, i) =>
      pt(i * 0.5, Math.sin(i / 40) * 30),
    );
    const idx = simplifyIndices(dense, 1);
    expect(idx.length).toBeLessThan(60);
    expect(idx.length).toBeGreaterThan(2);
  });

  it('returns indices in ascending order', () => {
    const curve = Array.from({ length: 80 }, (_, i) => pt(i, Math.cos(i / 7) * 15));
    const idx = simplifyIndices(curve, 0.8);
    const sorted = [...idx].sort((a, b) => a - b);
    expect(idx).toEqual(sorted);
  });

  it('handles a degenerate stroke where every point is identical', () => {
    const same = [pt(3, 3), pt(3, 3), pt(3, 3), pt(3, 3)];
    expect(simplifyIndices(same, 1)).toEqual([0, 3]);
  });
});

describe('simplify', () => {
  it('returns the kept points themselves', () => {
    const spike = [pt(0, 0), pt(1, 0), pt(2, 10), pt(3, 0), pt(4, 0)];
    expect(simplify(spike, 1)).toEqual([pt(0, 0), pt(2, 10), pt(4, 0)]);
  });
});
