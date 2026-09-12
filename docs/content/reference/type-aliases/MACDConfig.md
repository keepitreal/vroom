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

Source: [types/src/index.ts:849](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L849)

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

Source: [types/src/index.ts:851](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L851)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:853](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L853)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:892](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L892)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:897](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L897)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:885](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L885)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:890](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L890)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:879](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L879)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:866](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L866)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:870](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L870)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:868](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L868)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:861](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L861)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:857](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L857)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:872](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L872)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:863](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L863)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:876](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L876)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:874](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L874)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:855](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L855)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:859](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L859)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:900](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L900)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:902](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L902)

Draw the zero-reference line. Default true.
