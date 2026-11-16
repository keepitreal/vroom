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

Source: [types/src/index.ts:1389](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1389)

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

Source: [types/src/index.ts:1391](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1391)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1393](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1393)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1432](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1432)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1437](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1437)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1425](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1425)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1430](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1430)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1419](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1419)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1406](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1406)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1410](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1410)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1408](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1408)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1401](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1401)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1397](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1397)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1412](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1412)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1403](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1403)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1416](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1416)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1414](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1414)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1395](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1395)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1399](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1399)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1440](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1440)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1442](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1442)

Draw the zero-reference line. Default true.
