---
"@vroomchart/core-wasm": patch
"@vroomchart/react": patch
"react-native-vroom-chart": patch
---

RSI no longer pins to 100 across flat or zero-backfilled stretches. A window whose closes never moved is now undefined (drawn as a gap), and candles with a close of zero or below are treated as missing data: RSI warms up from the first real candle after them instead of counting the jump from 0 as a gain. The RSI trendline also resumes after a gap instead of stopping for the rest of the chart.
