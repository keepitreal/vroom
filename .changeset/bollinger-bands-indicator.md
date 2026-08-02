---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Add a Bollinger Bands overlay indicator (`bollingerBands` prop): a basis
SMA/EMA over a configurable period with upper/lower bands at ± N population
standard deviations, per-line colors/widths, and an optional translucent fill
between the bands. Defaults match TradingView (20-period SMA of close, ±2σ).
