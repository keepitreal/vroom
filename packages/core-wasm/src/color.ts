import type { VroomColor, VroomTheme } from '@vroomchart/types';
import { ColorKey, type VroomChartHandle } from './handle';

// Maps each VroomTheme field to its ColorKey. Mirror of
// packages/react-native/src/theme.ts (which keys off the same C enum).
export const COLOR_KEYS: Record<keyof VroomTheme, ColorKey> = {
  background: ColorKey.Background,
  bull: ColorKey.Bull,
  bear: ColorKey.Bear,
  grid: ColorKey.Grid,
  axisText: ColorKey.AxisText,
  crosshair: ColorKey.Crosshair,
  crosshairTarget: ColorKey.CrosshairTarget,
};

/**
 * Parse a color into a packed 0xAARRGGBB integer (Skia's ARGB order).
 *   - number → taken as already-packed ARGB
 *   - 6-digit hex → opaque (alpha forced to ff)
 *   - 8-digit hex → interpreted as AARRGGBB
 * Returns null for anything malformed so the caller can skip it.
 */
export function parseColor(value: VroomColor): number | null {
  if (typeof value === 'number') {
    return Number.isFinite(value) ? value >>> 0 : null;
  }
  let s = value.trim();
  if (s.startsWith('#')) s = s.slice(1);
  if (s.length === 6) s = `ff${s}`;
  if (s.length !== 8 || !/^[0-9a-fA-F]{8}$/.test(s)) return null;
  return parseInt(s, 16) >>> 0;
}

/** Push every provided theme color into the handle; skip unparseable ones. */
export function applyTheme(handle: VroomChartHandle, theme: VroomTheme): void {
  (Object.keys(COLOR_KEYS) as (keyof VroomTheme)[]).forEach((field) => {
    const value = theme[field];
    if (value == null) return;
    const argb = parseColor(value);
    if (argb == null) return;
    handle.setColor(COLOR_KEYS[field], argb);
  });
}

/** Split a packed 0xAARRGGBB integer into a CSS `rgba(...)` string. */
export function argbToCss(argb: number): string {
  const a = ((argb >>> 24) & 0xff) / 255;
  const r = (argb >>> 16) & 0xff;
  const g = (argb >>> 8) & 0xff;
  const b = argb & 0xff;
  return `rgba(${r}, ${g}, ${b}, ${a})`;
}
