# `ATRSmoothing`

```ts
type ATRSmoothing = "rma" | "sma" | "ema";
```

Source: [types/src/index.ts:1126](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1126)

How the true-range series is smoothed into ATR. `'rma'` is Wilder's original
(alpha = 1/period) and the conventional default; the other two are the
ordinary moving averages applied to the same series.

Distinct from [MAKind](MAKind.md), which the other indicators use — RMA is
specific to Wilder's indicators and isn't offered elsewhere.
