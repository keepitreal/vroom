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

Source: [types/src/index.ts:412](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L412)

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

Source: [types/src/index.ts:439](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L439)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:441](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L441)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:414](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L414)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:429](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L429)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:433](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L433)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:431](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L431)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:420](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L420)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:435](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L435)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:422](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L422)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:424](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L424)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:426](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L426)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:437](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L437)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:416](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L416)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:418](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L418)

Overbought band level (0..100). Default 70.
