# `PriceScaleMode`

```ts
type PriceScaleMode = "linear" | "log";
```

Source: [types/src/index.ts:242](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L242)

How prices map onto the y-axis.
'linear' — default: equal price differences take equal vertical distance.
'log' — logarithmic: equal price _ratios_ take equal vertical distance
(a move from 10 to 20 is as tall as 100 to 200). Suits long
lookbacks and assets that have moved by orders of magnitude.
