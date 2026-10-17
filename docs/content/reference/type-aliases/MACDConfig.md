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

Source: [types/src/index.ts:1294](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1294)

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

Source: [types/src/index.ts:1296](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1296)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1298](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1298)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1337](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1337)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1342](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1342)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1330](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1330)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1335](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1335)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1324](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1324)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1311](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1311)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1315](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1315)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1313](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1313)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1306](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1306)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1302](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1302)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1317](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1317)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1308](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1308)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1321](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1321)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1319](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1319)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1300](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1300)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1304](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1304)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1345](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1345)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1347](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1347)

Draw the zero-reference line. Default true.
