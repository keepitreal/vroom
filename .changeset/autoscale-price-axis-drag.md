---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Under `autoscaleY`, a drag on the price axis now scales how much room the fit leaves around the candles, and the fit goes on following them at that size as the view pans. Before, the drag did nothing on web and panned the chart on React Native. A double tap (double-click on web) on the price axis puts the price scale back to its default, with or without `autoscaleY`.
