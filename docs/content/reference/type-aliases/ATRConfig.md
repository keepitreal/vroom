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

Source: [types/src/index.ts:1131](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1131)

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

Source: [types/src/index.ts:1133](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1133)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1139](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1139)

Line color (hex string or packed ARGB number). Default teal.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1141](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1141)

Line stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:1135](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1135)

Lookback in candles. Default 14.

---

### smoothing?

```ts
optional smoothing?: ATRSmoothing;
```

Source: [types/src/index.ts:1137](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1137)

Smoothing applied to the true-range series. Default `'rma'`.
