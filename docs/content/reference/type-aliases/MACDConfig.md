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

Source: [types/src/index.ts:728](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L728)

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

Source: [types/src/index.ts:730](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L730)

Draw the pane. Default false.

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:732](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L732)

Fast moving-average length. Default 12.

---

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:771](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L771)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

---

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:776](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L776)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

---

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:764](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L764)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

---

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:769](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L769)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

---

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:758](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L758)

Draw the histogram bars. Default true.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:745](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L745)

MACD line color (hex string or packed ARGB number). Default blue.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:749](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L749)

Draw the MACD line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:747](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L747)

MACD line stroke width in px. Default 1.5.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:740](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L740)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:736](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L736)

Signal-line length. Default 9.

---

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:751](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L751)

Signal line color. Default orange.

---

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:742](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L742)

Averaging applied to the MACD series for the signal line. Default 'ema'.

---

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:755](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L755)

Draw the signal line. Default true.

---

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:753](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L753)

Signal line stroke width in px. Default 1.5.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:734](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L734)

Slow moving-average length (forced > fast). Default 26.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:738](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L738)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

---

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:779](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L779)

Zero-reference line color. Default gray.

---

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:781](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L781)

Draw the zero-reference line. Default true.
