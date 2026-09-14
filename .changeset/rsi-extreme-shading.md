---
'react-native-vroom-chart': minor
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
---

Shade the RSI pane where the reading runs past a band. The wash fades out at
the rule and deepens the further past it the line goes, so the size of an
excursion reads at a glance rather than just the fact of one — colored from the
theme's `accentBull` and `accentBear`, and never opaque enough to bury the line
or the rule. On by default; turn it off with `rsi.extremeFill: false`.
