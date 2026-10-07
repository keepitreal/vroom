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

Source: [types/src/index.ts:1187](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1187)

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

Source: [types/src/index.ts:1189](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1189)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1191](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1191)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1230](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1230)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1235](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1235)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1223](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1223)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1228](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1228)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1217](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1217)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1204](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1204)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1208](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1208)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1206](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1206)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1199](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1199)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1195](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1195)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1210](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1210)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1201](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1201)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1214](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1214)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1212](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1212)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1193](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1193)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1197](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1197)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1238](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1238)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1240](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1240)

Draw the zero-reference line. Default true.
