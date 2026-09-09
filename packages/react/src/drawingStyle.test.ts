import { describe, expect, it } from 'vitest';

import {
  DEFAULT_DRAW_COLOR,
  DEFAULT_DRAW_COLOR_HEX,
  DEFAULT_DRAW_WIDTH,
  newDrawingAttrs,
  resolveDrawingStyle,
} from './drawingStyle';

describe('resolveDrawingStyle', () => {
  it('defaults to the library blue at 2px with no fill', () => {
    expect(resolveDrawingStyle()).toEqual({
      color: DEFAULT_DRAW_COLOR_HEX,
      packed: DEFAULT_DRAW_COLOR,
      width: DEFAULT_DRAW_WIDTH,
    });
    expect(resolveDrawingStyle({})).toEqual(resolveDrawingStyle());
  });

  it('preserves a 6-digit host color so CSS swatches preview it', () => {
    const s = resolveDrawingStyle({ color: '#00FFFF' });
    expect(s.color).toBe('#00FFFF');
    expect(s.packed).toBe(0xff00ffff);
  });

  it('preserves 8-digit aarrggbb and packed ARGB as given', () => {
    expect(resolveDrawingStyle({ color: '#8000ffff' })).toMatchObject({
      color: '#8000ffff',
      packed: 0x8000ffff,
    });
    expect(resolveDrawingStyle({ color: 0xff00ffff })).toMatchObject({
      color: 0xff00ffff,
      packed: 0xff00ffff,
    });
  });

  it('falls back when the color is unparseable', () => {
    expect(resolveDrawingStyle({ color: 'nope' })).toEqual(resolveDrawingStyle());
  });

  it('uses a custom width and ignores non-positive values', () => {
    expect(resolveDrawingStyle({ width: 4 }).width).toBe(4);
    expect(resolveDrawingStyle({ width: 0 }).width).toBe(DEFAULT_DRAW_WIDTH);
    expect(resolveDrawingStyle({ width: -1 }).width).toBe(DEFAULT_DRAW_WIDTH);
  });

  it('passes through a box fill and drops an unparseable one', () => {
    expect(resolveDrawingStyle({ fill: '#5400ffff' }).fill).toBe('#5400ffff');
    expect(resolveDrawingStyle({ fill: 'nope' }).fill).toBeUndefined();
  });
});

describe('newDrawingAttrs', () => {
  it('stamps color and width onto every tool', () => {
    const style = { color: '#00FFFF' as const, width: 3 };
    expect(newDrawingAttrs(style, 'line')).toEqual({ color: '#00FFFF', width: 3 });
    expect(newDrawingAttrs(style, 'pencil')).toEqual({ color: '#00FFFF', width: 3 });
    expect(newDrawingAttrs(style, 'path')).toEqual({ color: '#00FFFF', width: 3 });
  });

  it('includes fill only on boxes', () => {
    const style = { color: '#00FFFF' as const, fill: '#3300ffff' as const };
    expect(newDrawingAttrs(style, 'box')).toEqual({
      color: '#00FFFF',
      width: DEFAULT_DRAW_WIDTH,
      fill: '#3300ffff',
    });
    expect(newDrawingAttrs(style, 'line').fill).toBeUndefined();
  });
});
