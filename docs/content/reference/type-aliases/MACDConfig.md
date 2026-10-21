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

Source: [types/src/index.ts:1319](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1319)

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

Source: [types/src/index.ts:1321](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1321)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1323](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1323)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1362](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1362)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1367](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1367)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1355](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1355)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1360](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1360)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1349](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1349)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1336](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1336)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1340](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1340)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1338](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1338)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1331](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1331)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1327](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1327)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1342](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1342)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1333](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1333)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1346](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1346)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1344](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1344)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1325](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1325)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1329](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1329)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1370](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1370)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1372](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1372)

Draw the zero-reference line. Default true.
