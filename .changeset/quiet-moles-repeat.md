---
'@vroomchart/core-wasm': patch
'@vroomchart/react': patch
'react-native-vroom-chart': patch
---

**The line chart's tip dot no longer pins itself to the right edge when you pan
into history.** It anchored to the newest *visible* close, so scrolling back
dragged the marker along with the viewport instead of leaving it on the candle
it belongs to. It now anchors to the newest close in the series and simply
scrolls off the pane with it, matching the current-price indicator.
