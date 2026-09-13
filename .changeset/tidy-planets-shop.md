---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add the Ichimoku Cloud indicator as a price-pane overlay via the new `ichimoku`
prop: Tenkan, Kijun, Senkou Span A/B and Chikou, with the cloud shaded between
the leading spans and tinted per direction, splitting cleanly at each crossover.

Each of the five lines takes its own color, width and visibility, and the cloud
takes a color per direction plus an opacity.

The leading spans plot 26 bars past the newest candle by default, so the chart
now reserves that much empty time on the right when it frames itself, and
enabling the indicator later pulls the view forward to match.
