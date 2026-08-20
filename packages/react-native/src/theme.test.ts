import { describe, expect, it, vi } from 'vitest';

import { applyTheme, COLOR_KEYS, FLOAT_KEYS, parseColor } from './theme';
import type { ChartHandle } from './jsi.d';

describe('parseColor', () => {
  it('expands 6-digit hex to opaque ARGB', () => {
    expect(parseColor('#0d1117')).toBe(0xff0d1117);
    expect(parseColor('0d1117')).toBe(0xff0d1117); // '#' optional
  });

  it('parses 8-digit hex as AARRGGBB', () => {
    expect(parseColor('#80ff0000')).toBe(0x80ff0000);
  });

  it('passes a number through as packed ARGB', () => {
    expect(parseColor(0xff26a69a)).toBe(0xff26a69a);
  });

  it('is case-insensitive and trims whitespace', () => {
    expect(parseColor('  #AABBCC  ')).toBe(0xffaabbcc);
  });

  it('returns null for malformed input', () => {
    expect(parseColor('#12')).toBeNull();
    expect(parseColor('#zzzzzz')).toBeNull();
    expect(parseColor('not-a-color')).toBeNull();
    expect(parseColor(Number.NaN)).toBeNull();
  });
});

describe('applyTheme', () => {
  const fakeHandle = () => {
    const calls: Array<[number, number]> = [];
    const handle = {
      setColor: vi.fn((key: number, argb: number) => {
        calls.push([key, argb]);
      }),
    } as unknown as ChartHandle;
    return { handle, calls };
  };

  it('maps each field to the right VroomColorKey index', () => {
    const { handle, calls } = fakeHandle();
    applyTheme(handle, {
      background: '#0d1117',
      bull: '#26a69a',
      crosshairTarget: '#ffffff',
    });
    expect(calls).toContainEqual([COLOR_KEYS.background, 0xff0d1117]);
    expect(calls).toContainEqual([COLOR_KEYS.bull, 0xff26a69a]);
    expect(calls).toContainEqual([COLOR_KEYS.crosshairTarget, 0xffffffff]);
    expect(calls).toHaveLength(3);
  });

  it('skips absent and unparseable colors', () => {
    const { handle, calls } = fakeHandle();
    applyTheme(handle, { grid: 'nonsense', axisText: '#c9d1d9' });
    expect(calls).toEqual([[COLOR_KEYS.axisText, 0xffc9d1d9]]);
  });

  it('pushes numeric fields through setFloat at their VroomFloatKey index', () => {
    // The indices are hand-kept in step with the C++ enum, so pin the literals.
    const floats: Array<[number, number]> = [];
    const handle = {
      setColor: vi.fn(),
      setFloat: vi.fn((key: number, value: number) => {
        floats.push([key, value]);
      }),
    } as unknown as ChartHandle;
    applyTheme(handle, { lineWidth: 2.5, lineTension: 0.5 });
    expect(FLOAT_KEYS.lineTension).toBe(13);
    expect(floats).toContainEqual([11, 2.5]);
    expect(floats).toContainEqual([13, 0.5]);
    expect(floats).toHaveLength(2);
  });

  it('does nothing for an empty theme', () => {
    const { handle, calls } = fakeHandle();
    applyTheme(handle, {});
    expect(calls).toHaveLength(0);
  });
});
