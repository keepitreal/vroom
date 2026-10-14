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

Source: [types/src/index.ts:1274](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1274)

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

Source: [types/src/index.ts:1276](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1276)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1282](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1282)

Line color (hex string or packed ARGB number). Default teal.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1284)

Line stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:1278](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1278)

Lookback in candles. Default 14.

---

### smoothing?

```ts
optional smoothing?: ATRSmoothing;
```

Source: [types/src/index.ts:1280](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1280)

Smoothing applied to the true-range series. Default `'rma'`.
