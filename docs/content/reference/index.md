# react-native-vroom-chart

## Type Aliases

| Type Alias | Description |
| ------ | ------ |
| [BollingerBandsConfig](type-aliases/BollingerBandsConfig.md) | Bollinger Bands overlay config. A basis moving average of `source` over `period`, banded at ± `stdDev` × population standard deviation of the same window, drawn as three lines on the price pane with an optional translucent fill between the bands. No pane is reserved. |
| [Candle](type-aliases/Candle.md) | A single OHLCV bar. `candles` is an array of these. |
| [ChartType](type-aliases/ChartType.md) | How the price series is drawn. 'candles' — default: candlestick bodies + wicks. 'line' — a single polyline through each candle's close. Volume, indicators, overlays, crosshair, and drawings still render. |
| [CrosshairEvent](type-aliases/CrosshairEvent.md) | Payload passed to `onCrosshair` as the crosshair shows, moves, or hides. |
| [MACDConfig](type-aliases/MACDConfig.md) | MACD indicator config. Rendered in its own pane below the candles. |
| [MASource](type-aliases/MASource.md) | Price source for a moving average. |
| [MovingAverageOverlay](type-aliases/MovingAverageOverlay.md) | A moving-average overlay line drawn on the price pane. Provide an array of these via `movingAverages` to render a ribbon of SMA/EMA lines. |
| [RSIConfig](type-aliases/RSIConfig.md) | RSI indicator config. Rendered in a pane below the candles when enabled. |
| [VisibleRange](type-aliases/VisibleRange.md) | A time window over the candle data, as Unix epoch milliseconds. |
| [VroomChartProps](type-aliases/VroomChartProps.md) | Props for the [VroomChart](functions/VroomChart.md) component. The cross-platform props come from VroomChartCoreProps; `style` is the React Native flavor. |
| [VroomColor](type-aliases/VroomColor.md) | A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a packed ARGB number. In `VroomTheme` every field is optional — omitted colors keep the library default. |
| [VroomTheme](type-aliases/VroomTheme.md) | Color overrides for the chart, passed via the `theme` prop. |
| [VWAPConfig](type-aliases/VWAPConfig.md) | VWAP overlay config (session anchor). Drawn as a single line on the price pane, resetting each session. |

## Functions

| Function | Description |
| ------ | ------ |
| [VroomChart](functions/VroomChart.md) | Skia-rendered candlestick chart. Pass OHLCV `candles` and size it via `style` (it fills its parent by default). Pan to scroll, pinch to zoom, drag the price/time axes to rescale, and long-press for the crosshair. Optional indicators (`rsi`, `macd`, `movingAverages`, `vwap`), colors (`theme`), and events (`onCrosshair`, `onViewportChange`) are configured through props. |
