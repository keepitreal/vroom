import { describe, expect, it } from 'vitest';
import type { TransitionEasing } from '@vroomchart/types';

import { ease, easingIndex } from './easing';

const KINDS: TransitionEasing[] = ['linear', 'ease-in', 'ease-out', 'ease-in-out'];
const SAMPLES = Array.from({ length: 21 }, (_, i) => i / 20);

describe('ease', () => {
  it('pins the endpoints so a transition starts and finishes exactly', () => {
    for (const kind of KINDS) {
      expect(ease(kind, 0)).toBe(0);
      expect(ease(kind, 1)).toBe(1);
    }
  });

  it('is monotonically increasing', () => {
    for (const kind of KINDS) {
      for (let i = 1; i < SAMPLES.length; i++) {
        expect(ease(kind, SAMPLES[i])).toBeGreaterThan(ease(kind, SAMPLES[i - 1]));
      }
    }
  });

  it('leaves linear progress untouched', () => {
    for (const p of SAMPLES) expect(ease('linear', p)).toBe(p);
  });

  it('ease-in-out is the smoothstep the candle↔line morph already used', () => {
    for (const p of SAMPLES) {
      expect(ease('ease-in-out', p)).toBeCloseTo(p * p * (3 - 2 * p), 12);
    }
  });

  it('ease-in-out is symmetric about its midpoint', () => {
    expect(ease('ease-in-out', 0.5)).toBeCloseTo(0.5, 12);
    for (const p of SAMPLES) {
      expect(ease('ease-in-out', p)).toBeCloseTo(1 - ease('ease-in-out', 1 - p), 12);
    }
  });

  it('ease-in starts slow and ease-out starts fast', () => {
    for (const p of SAMPLES.slice(1, -1)) {
      expect(ease('ease-in', p)).toBeLessThan(p);
      expect(ease('ease-out', p)).toBeGreaterThan(p);
    }
  });

  it('falls back to ease-in-out for an unset or unknown curve', () => {
    for (const p of SAMPLES) {
      expect(ease(undefined, p)).toBe(ease('ease-in-out', p));
      expect(ease('bogus' as TransitionEasing, p)).toBe(ease('ease-in-out', p));
    }
  });
});

describe('easingIndex', () => {
  it('matches the VroomEasing order the core declares', () => {
    expect(KINDS.map(easingIndex)).toEqual([0, 1, 2, 3]);
  });

  it('falls back to ease-in-out for an unset or unknown curve', () => {
    expect(easingIndex(undefined)).toBe(easingIndex('ease-in-out'));
    expect(easingIndex('bogus' as TransitionEasing)).toBe(easingIndex('ease-in-out'));
  });
});
