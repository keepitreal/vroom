---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Expand the MACD config beyond its four lengths.

Inputs gain a price `source` and an SMA/EMA choice for the fast and slow legs
(`maType`) and for the signal line (`signalMaType`). Styling gains per-series
color, width, and visibility for the MACD and signal lines, four histogram
colors (above and below zero, each with a shade for bars building and a lighter
one for bars easing back toward zero), and a configurable zero line. Hiding a
series rescales the pane around what is left on show.

Every new field is optional and falls back to the previous look, so existing
charts render unchanged: histogram bars still follow `theme.accentBull` /
`theme.accentBear`, and an unset fading color derives from its base at half
opacity.

The low-level `VroomChartHandle.setMACD` now takes a single spec object instead
of four positional arguments. Consumers of the `<VroomChart>` components are
unaffected.
