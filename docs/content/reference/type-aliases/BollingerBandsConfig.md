# `BollingerBandsConfig`

```ts
type BollingerBandsConfig = {
  basis?: "sma" | "ema";
  enabled?: boolean;
  fill?: boolean;
  fillOpacity?: number;
  lowerColor?: string | number;
  lowerWidth?: number;
  middleColor?: string | number;
  middleWidth?: number;
  period?: number;
  source?: MASource;
  stdDev?: number;
  upperColor?: string | number;
  upperWidth?: number;
};
```

Source: [types/src/index.ts:317](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L317)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### basis?

```ts
optional basis?: "sma" | "ema";
```

Source: [types/src/index.ts:329](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L329)

Basis (middle) line type. Default 'sma'. The stdev always uses the
window's arithmetic mean, even with an EMA basis (TradingView semantics).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:318](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L318)

---

### fill?

```ts
optional fill?: boolean;
```

Source: [types/src/index.ts:343](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L343)

Translucent fill between the bands. Default true.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:345](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L345)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:339](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L339)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:341](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L341)

Lower band stroke width in px. Default 1.

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:335](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L335)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:337](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L337)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:320](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L320)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:324](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L324)

Price source. Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:322](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L322)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:331](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L331)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:333](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L333)

Upper band stroke width in px. Default 1.
