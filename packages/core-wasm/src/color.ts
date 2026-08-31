import type { VroomColor, VroomTheme } from '@vroomchart/types';
import { ColorKey, FloatKey, type VroomChartHandle } from './handle';

// Maps each color VroomTheme field to its ColorKey. Mirror of
// packages/react-native/src/theme.ts (which keys off the same C enum).
export const COLOR_KEYS: Partial<Record<keyof VroomTheme, ColorKey>> = {
  background: ColorKey.Background,
  bull: ColorKey.Bull,
  bear: ColorKey.Bear,
  grid: ColorKey.Grid,
  axisText: ColorKey.AxisText,
  badgeText: ColorKey.BadgeText,
  crosshair: ColorKey.Crosshair,
  crosshairTarget: ColorKey.CrosshairTarget,
  borderBull: ColorKey.BorderBull,
  borderBear: ColorKey.BorderBear,
  wickBull: ColorKey.WickBull,
  wickBear: ColorKey.WickBear,
  accentBull: ColorKey.AccentBull,
  accentBear: ColorKey.AccentBear,
  lineColor: ColorKey.Line,
};

// Maps each numeric VroomTheme field to its FloatKey.
export const FLOAT_KEYS: Partial<Record<keyof VroomTheme, FloatKey>> = {
  wickWidth: FloatKey.WickWidth,
  candleRadius: FloatKey.CandleRadius,
  volumeRadius: FloatKey.VolumeRadius,
  lineWidth: FloatKey.LineWidth,
  lineGradientOpacity: FloatKey.LineGradientOpacity,
  lineTension: FloatKey.LineTension,
};

// Maps each boolean VroomTheme field to its FloatKey (pushed as 0/1).
//
// `showXAxis` / `showYAxis` are absent on purpose: they animate, so the wrapper
// drives them as a collapse scalar through setAxisCollapse. A float slot here
// would let this sweep snap them behind the animation's back.
export const BOOL_KEYS: Partial<Record<keyof VroomTheme, FloatKey>> = {
  wickRoundCap: FloatKey.WickRoundCap,
  lineTipDot: FloatKey.LineTipDot,
  lineTipPulse: FloatKey.LineTipPulse,
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

/** Push every provided theme color + numeric float into the handle; skip bad ones. */
export function applyTheme(handle: VroomChartHandle, theme: VroomTheme): void {
  (Object.keys(COLOR_KEYS) as (keyof VroomTheme)[]).forEach((field) => {
    const value = theme[field];
    if (typeof value !== 'string' && typeof value !== 'number') return;
    const argb = parseColor(value);
    if (argb == null) return;
    handle.setColor(COLOR_KEYS[field]!, argb);
  });
  (Object.keys(FLOAT_KEYS) as (keyof VroomTheme)[]).forEach((field) => {
    const value = theme[field];
    if (typeof value !== 'number' || !Number.isFinite(value)) return;
    handle.setFloat(FLOAT_KEYS[field]!, value);
  });
  (Object.keys(BOOL_KEYS) as (keyof VroomTheme)[]).forEach((field) => {
    const value = theme[field];
    if (typeof value !== 'boolean') return;
    handle.setFloat(BOOL_KEYS[field]!, value ? 1 : 0);
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
