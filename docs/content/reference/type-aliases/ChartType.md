# Type Alias: ChartType

```ts
type ChartType = "candles" | "line";
```

Defined in: [types/src/index.ts:128](https://github.com/keepitreal/vroom/blob/b705ea8b686da312da84b1a125fe7a6c2df5261e/packages/types/src/index.ts#L128)

How the price series is drawn.
  'candles' — default: candlestick bodies + wicks.
  'line'    — a single polyline through each candle's close. Volume, indicators,
              overlays, crosshair, and drawings still render.
