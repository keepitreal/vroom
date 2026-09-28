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

Source: [types/src/index.ts:1084](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1084)

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

Source: [types/src/index.ts:1086](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1086)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1088](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1088)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1127](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1127)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1132](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1132)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1120](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1120)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1125](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1125)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1114](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1114)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1101](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1101)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1105](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1105)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1103](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1103)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1096](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1096)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1092](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1092)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1107](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1107)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1098](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1098)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1111](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1111)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1109](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1109)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1090](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1090)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1094](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1094)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1135](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1135)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1137](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1137)

Draw the zero-reference line. Default true.
