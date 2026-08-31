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

Source: [types/src/index.ts:419](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L419)

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

Source: [types/src/index.ts:446](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L446)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:448](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L448)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:421](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L421)

Draw the pane. Default false.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:436](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L436)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:440](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L440)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:438](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L438)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:427](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L427)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:442](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L442)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:429](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L429)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:431](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L431)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:433](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L433)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:444](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L444)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:423](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L423)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:425](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L425)

Overbought band level (0..100). Default 70.
