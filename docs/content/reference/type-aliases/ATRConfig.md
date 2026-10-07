# `ATRConfig`

```ts
type ATRConfig = {
  enabled?: boolean;
  lineColor?: string | number;
  lineWidth?: number;
  period?: number;
  smoothing?: ATRSmoothing;
};
```

Source: [types/src/index.ts:1263](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1263)

ATR (Average True Range) indicator config. Rendered in its own pane below the
candles: a single line measuring volatility in price units.

True Range is the widest of the bar's own high-low span and the two gaps from
its extremes to the previous close, so an overnight jump the bar's range
misses still counts. ATR smooths that series over `period` bars. It is
strictly positive and unbounded, so the pane fits 0..peak from its bottom
edge rather than centering on a reference level.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:1265](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1265)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1271](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1271)

Line color (hex string or packed ARGB number). Default teal.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1273](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1273)

Line stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:1267](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1267)

Lookback in candles. Default 14.

---

### smoothing?

```ts
optional smoothing?: ATRSmoothing;
```

Source: [types/src/index.ts:1269](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1269)

Smoothing applied to the true-range series. Default `'rma'`.
