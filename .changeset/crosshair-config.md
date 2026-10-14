---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

Add a `crosshair` prop that takes a mode (`'advanced' | 'simple' | 'none'`) or a `CrosshairConfig`.

- `'advanced'` (default) is the existing crosshair.
- `'simple'` is a sparkline scrubber: a vertical line that follows x, a time badge at `simple.height` (0 = bottom, 1 = top), and dimming to the right of the line (`simple.dimAhead`, `simple.dimOpacity`).
- `'none'` stops hover, long-press and press-drag from raising the crosshair or firing `onCrosshair`, and dismisses one that's showing. Pan and zoom still work, and a `crosshairOverride` still draws.

The config also sets `lineStyle` (`'dashed' | 'dotted' | 'solid'`), `lineWidth`, `timeFormat` (`'datetime' | 'time' | 'date'`) and `hour12`. `crosshairOffset` and `crosshairButton` are deprecated in favor of `crosshair.offset` and `crosshair.button`.
