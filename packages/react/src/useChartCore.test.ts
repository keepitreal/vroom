import { describe, expect, it } from 'vitest';
import { resolveCrosshair } from './useChartCore';

describe('resolveCrosshair', () => {
  it('leaves snapToData off unless it is set', () => {
    expect(resolveCrosshair(undefined, undefined, undefined).spec.snapToData).toBe(false);
    expect(resolveCrosshair('simple', undefined, undefined).spec.snapToData).toBe(false);
    expect(resolveCrosshair({ mode: 'simple' }, undefined, undefined).spec.snapToData).toBe(false);
  });

  it('passes snapToData through', () => {
    expect(
      resolveCrosshair({ mode: 'simple', snapToData: true }, undefined, undefined).spec.snapToData,
    ).toBe(true);
  });
});
