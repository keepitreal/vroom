# `ChartType`

```ts
type ChartType = "candles" | "line";
```

Source: [types/src/index.ts:134](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L134)

How the price series is drawn.
'candles' — default: candlestick bodies + wicks.
'line' — a single polyline through each candle's close. Volume, indicators,
overlays, crosshair, and drawings still render.
