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

Source: [types/src/index.ts:1036](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1036)

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

Source: [types/src/index.ts:1038](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1038)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1040](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1040)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1079](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1079)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1084](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1084)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1072](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1072)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1077](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1077)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1066](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1066)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1053](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1053)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1057](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1057)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1055](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1055)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1048](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1048)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1044](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1044)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1059](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1059)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1050](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1050)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1063](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1063)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1061](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1061)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1042](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1042)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1046](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1046)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1087](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1087)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1089](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1089)

Draw the zero-reference line. Default true.
