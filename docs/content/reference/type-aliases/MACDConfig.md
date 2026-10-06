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

Source: [types/src/index.ts:1174](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1174)

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

Source: [types/src/index.ts:1176](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1176)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1178](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1178)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1217](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1217)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1222](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1222)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1210](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1210)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1215](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1215)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1204](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1204)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1191](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1191)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1195](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1195)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1193](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1193)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1186](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1186)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1182](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1182)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1197](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1197)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1188](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1188)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1201](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1201)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1199](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1199)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1180](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1180)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1184](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1184)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1225](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1225)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1227](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1227)

Draw the zero-reference line. Default true.
