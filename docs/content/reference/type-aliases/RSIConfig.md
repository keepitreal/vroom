# `RSIConfig`

```ts
type RSIConfig = {
  bandColor?: string | number;
  bandsVisible?: boolean;
  enabled?: boolean;
  lineColor?: string | number;
  lineVisible?: boolean;
  lineWidth?: number;
  lowerBand?: number;
  maColor?: string | number;
  maPeriod?: number;
  maType?: MAKind;
  maVisible?: boolean;
  maWidth?: number;
  period?: number;
  upperBand?: number;
};
```

Source: [types/src/index.ts:309](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L309)

RSI indicator config. Rendered in a pane below the candles when enabled: the
RSI line, an optional moving-average trendline over it, and two dashed rules
at the overbought and oversold levels.

RSI reads closes only — Wilder's definition is built on close-to-close
change — so unlike the moving-average, Bollinger, and MACD configs it takes
no [MASource](MASource.md).

## Properties

### bandColor?

```ts
optional bandColor?: string | number;
```

Source: [types/src/index.ts:336](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L336)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:338](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L338)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:311](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L311)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:326](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L326)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:330](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L330)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:328](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L328)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:317](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L317)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:332](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L332)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:319](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L319)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:321](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L321)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:323](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L323)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:334](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L334)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:313](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L313)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:315](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L315)

Overbought band level (0..100). Default 70.
