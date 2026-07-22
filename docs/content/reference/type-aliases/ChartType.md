# Type Alias: ChartType

```ts
type ChartType = "candles" | "line";
```

Defined in: [types/src/index.ts:128](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L128)

How the price series is drawn.
  'candles' — default: candlestick bodies + wicks.
  'line'    — a single polyline through each candle's close. Volume, indicators,
              overlays, crosshair, and drawings still render.
