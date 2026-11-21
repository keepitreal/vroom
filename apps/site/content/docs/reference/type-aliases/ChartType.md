---
title: "ChartType"
description: "How the price series is drawn."
---

```ts
type ChartType = "candles" | "line";
```

Source: [types/src/index.ts:305](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L305)

How the price series is drawn.
  'candles' — default: candlestick bodies + wicks.
  'line'    — a single polyline through each candle's close. Volume, indicators,
              overlays, crosshair, and drawings still render.
