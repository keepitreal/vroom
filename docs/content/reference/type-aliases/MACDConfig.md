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

Source: [types/src/index.ts:930](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L930)

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

Source: [types/src/index.ts:932](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L932)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:934](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L934)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:973](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L973)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:978](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L978)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:966](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L966)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:971](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L971)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:960](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L960)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:947](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L947)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:951](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L951)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:949](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L949)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:942](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L942)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:938](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L938)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:953](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L953)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:944](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L944)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:957](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L957)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:955](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L955)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:936](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L936)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:940](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L940)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:981](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L981)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:983](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L983)

Draw the zero-reference line. Default true.
