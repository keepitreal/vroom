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

Source: [types/src/index.ts:348](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L348)

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

Source: [types/src/index.ts:375](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L375)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:377](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L377)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:350](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L350)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:365](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L365)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:369](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L369)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:367](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L367)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:356](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L356)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:371](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L371)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:358](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L358)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:360](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L360)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:362](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L362)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:373](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L373)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:352](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L352)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:354](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L354)

Overbought band level (0..100). Default 70.
