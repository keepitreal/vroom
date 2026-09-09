import type { DefaultDrawingStyle, Drawing, VroomColor } from '@vroomchart/types';
import { parseColor } from '@vroomchart/core-wasm';

// Library default: the same packed ARGB the core uses when a drawing omits
// `color`. Stamped as an 8-digit `#aarrggbb` string so a host swatch has
// *a* value — hosts that want CSS-friendly 6-digit hex pass `drawingStyle.color`.
export const DEFAULT_DRAW_COLOR = 0xff2962ff;
export const DEFAULT_DRAW_COLOR_HEX = '#ff2962ff';
export const DEFAULT_DRAW_WIDTH = 2;

export type ResolvedDrawingStyle = {
  /** Value stamped onto the committed drawing (host-supplied, else the default hex). */
  color: VroomColor;
  /** Packed ARGB for the live draft preview. */
  packed: number;
  width: number;
  fill?: VroomColor;
};

/**
 * Resolve `drawingStyle` into the stroke/fill used for a new drawing. Invalid
 * colors fall back to the library default so a bad host value can't blank the
 * stroke; an unparseable `fill` is dropped so boxes keep the 10% stroke tint.
 */
export function resolveDrawingStyle(
  style?: DefaultDrawingStyle | null,
): ResolvedDrawingStyle {
  const parsed = style?.color != null ? parseColor(style.color) : null;
  const packed = parsed ?? DEFAULT_DRAW_COLOR;
  const color: VroomColor =
    parsed != null && style?.color != null ? style.color : DEFAULT_DRAW_COLOR_HEX;
  const width =
    typeof style?.width === 'number' && Number.isFinite(style.width) && style.width > 0
      ? style.width
      : DEFAULT_DRAW_WIDTH;
  const fillParsed = style?.fill != null ? parseColor(style.fill) : null;
  return {
    color,
    packed,
    width,
    ...(fillParsed != null && style?.fill != null ? { fill: style.fill } : {}),
  };
}

/** Attributes stamped onto a newly created drawing of `type`. */
export function newDrawingAttrs(
  style: DefaultDrawingStyle | null | undefined,
  type: Drawing['type'],
): { color: VroomColor; width: number; fill?: VroomColor } {
  const s = resolveDrawingStyle(style);
  return {
    color: s.color,
    width: s.width,
    ...(type === 'box' && s.fill != null ? { fill: s.fill } : {}),
  };
}
