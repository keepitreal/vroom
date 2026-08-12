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
  macdColor?: string | number;
  macdVisible?: boolean;
  macdWidth?: number;
  maType?: "sma" | "ema";
  signal?: number;
  signalColor?: string | number;
  signalMaType?: "sma" | "ema";
  signalVisible?: boolean;
  signalWidth?: number;
  slow?: number;
  source?: MASource;
  zeroLineColor?: string | number;
  zeroLineVisible?: boolean;
};
```

Source: [types/src/index.ts:518](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L518)

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

Source: [types/src/index.ts:519](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L519)

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:521](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L521)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:560](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L560)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:565](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L565)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:553](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L553)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:558](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L558)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:547](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L547)

Draw the histogram bars. Default true.

---

### macdColor?

```ts
optional macdColor?: string | number;
```

Source: [types/src/index.ts:534](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L534)

MACD line color (hex string or packed ARGB number). Default blue.

---

### macdVisible?

```ts
optional macdVisible?: boolean;
```

Source: [types/src/index.ts:538](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L538)

Draw the MACD line. Default true.

---

### macdWidth?

```ts
optional macdWidth?: number;
```

Source: [types/src/index.ts:536](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L536)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: "sma" | "ema";
```

Source: [types/src/index.ts:529](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L529)

Averaging used for the fast and slow legs. Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:525](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L525)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:540](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L540)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: "sma" | "ema";
```

Source: [types/src/index.ts:531](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L531)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:544](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L544)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:542](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L542)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:523](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L523)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:527](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L527)

Price source for the fast/slow legs. Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:568](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L568)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:570](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L570)

Draw the zero-reference line. Default true.
