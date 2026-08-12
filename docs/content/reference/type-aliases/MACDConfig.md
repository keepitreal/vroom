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

Source: [types/src/index.ts:571](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L571)

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

Source: [types/src/index.ts:573](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L573)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:575](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L575)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:614](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L614)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:619](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L619)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:607](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L607)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:612](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L612)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:601](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L601)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:588](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L588)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:592](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L592)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:590](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L590)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:583](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L583)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:579](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L579)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:594](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L594)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:585](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L585)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:598](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L598)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:596](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L596)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:577](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L577)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:581](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L581)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:622](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L622)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:624](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L624)

Draw the zero-reference line. Default true.
