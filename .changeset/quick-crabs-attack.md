---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add `theme.showXAxis` / `theme.showYAxis` to hide either axis.

Hiding an axis collapses its strip and hands the reclaimed space to the plot, so
the candles grow to fill it. The strip's contents — labels, the current-price
badge, the crosshair and price-line badges — fade out over the first half of the
collapse, so text is never squeezed into a strip too narrow to hold it. Both
default to `true` and animate over `transitionMs`, snapping under reduced motion.

Also fixes the volume-collapse animation not handing back to the render loop
when it finishes, which could leave the line-tip pulse frozen after toggling
volume — the same defect fixed for the candle↔line morph in #50.
