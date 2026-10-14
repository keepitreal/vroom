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

Source: [types/src/index.ts:1267](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1267)

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

Source: [types/src/index.ts:1269](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1269)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1271](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1271)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1310](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1310)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1315](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1315)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1303](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1303)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1308](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1308)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1297](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1297)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1284)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1288)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1286](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1286)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1279](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1279)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1275](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1275)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1290](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1290)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1281](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1281)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1294](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1294)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1292](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1292)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1273](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1273)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1277](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1277)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1318](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1318)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1320](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1320)

Draw the zero-reference line. Default true.
