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

Source: [types/src/index.ts:854](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L854)

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

Source: [types/src/index.ts:856](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L856)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:858](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L858)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:897](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L897)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:902](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L902)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:890](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L890)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:895](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L895)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:884](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L884)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:871](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L871)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:875](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L875)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:873](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L873)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:866](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L866)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:862](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L862)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:877](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L877)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:868](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L868)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:881](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L881)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:879](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L879)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:860](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L860)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:864](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L864)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:905](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L905)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:907](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L907)

Draw the zero-reference line. Default true.
