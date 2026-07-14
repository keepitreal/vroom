import type { ChartHandle } from './jsi.d';
import type { VroomColor, VroomTheme } from './types';

// Maps each color VroomTheme field to its VroomColorKey index in the C++ enum
// (packages/core/include/vroom/vroom_chart.h). Keep in sync with that enum;
// new keys are appended there so existing indices never shift.
export const COLOR_KEYS: Partial<Record<keyof VroomTheme, number>> = {
  background: 0, // VROOM_COLOR_BACKGROUND
  bull: 1, // VROOM_COLOR_BULL
  bear: 2, // VROOM_COLOR_BEAR
  grid: 4, // VROOM_COLOR_GRID
  axisText: 5, // VROOM_COLOR_AXIS_TEXT
  crosshair: 6, // VROOM_COLOR_CROSSHAIR
  crosshairTarget: 9, // VROOM_COLOR_CROSSHAIR_TARGET
  borderBull: 10, // VROOM_COLOR_BORDER_BULL
  borderBear: 11, // VROOM_COLOR_BORDER_BEAR
  wickBull: 12, // VROOM_COLOR_WICK_BULL
  wickBear: 13, // VROOM_COLOR_WICK_BEAR
  accentBull: 14, // VROOM_COLOR_ACCENT_BULL
  accentBear: 15, // VROOM_COLOR_ACCENT_BEAR
};

// Maps each numeric VroomTheme field to its VroomFloatKey index.
export const FLOAT_KEYS: Partial<Record<keyof VroomTheme, number>> = {
  wickWidth: 1, // VROOM_FLOAT_WICK_WIDTH_PX
  candleRadius: 8, // VROOM_FLOAT_CANDLE_RADIUS_PX
  volumeRadius: 10, // VROOM_FLOAT_VOLUME_RADIUS_PX
};

// Maps each boolean VroomTheme field to its VroomFloatKey index (pushed as 0/1).
export const BOOL_KEYS: Partial<Record<keyof VroomTheme, number>> = {
  wickRoundCap: 9, // VROOM_FLOAT_WICK_ROUND_CAP
};

// Parses a color into a packed 0xAARRGGBB integer (Skia's ARGB order).
//   - number → taken as already-packed ARGB
//   - '#rgb'-style 6-digit hex → opaque (alpha forced to ff)
//   - 8-digit hex → interpreted as AARRGGBB
// Returns null for anything malformed so the caller can skip it.
export function parseColor(value: VroomColor): number | null {
  if (typeof value === 'number') {
    return Number.isFinite(value) ? value >>> 0 : null;
  }
  let s = value.trim();
  if (s.startsWith('#')) s = s.slice(1);
  if (s.length === 6) s = `ff${s}`; // assume opaque
  if (s.length !== 8 || !/^[0-9a-fA-F]{8}$/.test(s)) return null;
  return parseInt(s, 16) >>> 0;
}

// Pushes every provided theme color + numeric float into the chart core via
// handle.setColor / handle.setFloat. Unspecified or unparseable values are
// skipped (they keep their default).
export function applyTheme(handle: ChartHandle, theme: VroomTheme): void {
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
