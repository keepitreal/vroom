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

Source: [types/src/index.ts:466](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L466)

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

Source: [types/src/index.ts:493](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L493)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:495](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L495)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:468](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L468)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:483](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L483)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:487](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L487)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:485](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L485)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:474](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L474)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:489](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L489)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:476](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L476)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:478](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L478)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:480](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L480)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:491](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L491)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:470](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L470)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:472](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L472)

Overbought band level (0..100). Default 70.
