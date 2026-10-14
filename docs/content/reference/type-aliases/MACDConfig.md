# `MACDConfig`

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  histogramDownColor?: string | number;
  histogramDownFadingColor?: string | number;
  histogramUpColor?: string | number;
  histogramUpFadingColor?: string | number;
  histogramVisible?: boolean;
  lineColor?: string | number;
  lineVisible?: boolean;
  lineWidth?: number;
  maType?: MAKind;
  signal?: number;
  signalColor?: string | number;
  signalMaType?: MAKind;
  signalVisible?: boolean;
  signalWidth?: number;
  slow?: number;
  source?: MASource;
  zeroLineColor?: string | number;
  zeroLineVisible?: boolean;
};
```

Source: [types/src/index.ts:1261](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1261)

MACD indicator config. Rendered in its own pane below the candles: the gap
between a fast and a slow moving average, a signal line smoothing that gap,
and a histogram of the distance between the two.

Every style field is optional and falls back to the stock look, so an
untouched config renders exactly as it always has.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:1263](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1263)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1265](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1265)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1304](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1304)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1309](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1309)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1297](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1297)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1302](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1302)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1291](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1291)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1278](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1278)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1282](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1282)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1280](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1280)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1273](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1273)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1269](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1269)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1284)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1275](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1275)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1288)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1286](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1286)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1267](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1267)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1271](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1271)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1312](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1312)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1314](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1314)

Draw the zero-reference line. Default true.
