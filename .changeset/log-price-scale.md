---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Add a `priceScaleMode` prop (`'linear' | 'log'`) for a logarithmic price axis. In `'log'` mode equal price ratios take equal vertical distance, and pan, zoom, axis drag and auto-fit all work in log space. The y-axis labels follow TradingView's log tick spacing (e.g. 10k / 20k / 50k / 100k). The host app controls the mode; switching snaps.
