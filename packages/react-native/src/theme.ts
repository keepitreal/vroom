import type { ChartHandle } from './jsi.d';
import type { VroomColor, VroomTheme } from './types';

// Maps each VroomTheme field to its VroomColorKey index in the C++ enum
// (packages/core/include/vroom/vroom_chart.h). Keep in sync with that enum;
// new keys are appended there so existing indices never shift.
export const COLOR_KEYS: Record<keyof VroomTheme, number> = {
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

// Pushes every provided theme color into the chart core via handle.setColor.
// Unspecified or unparseable colors are skipped (they keep their default).
export function applyTheme(handle: ChartHandle, theme: VroomTheme): void {
  (Object.keys(COLOR_KEYS) as (keyof VroomTheme)[]).forEach((field) => {
    const value = theme[field];
    if (value == null) return;
    const argb = parseColor(value);
    if (argb == null) return;
    handle.setColor(COLOR_KEYS[field], argb);
  });
}
