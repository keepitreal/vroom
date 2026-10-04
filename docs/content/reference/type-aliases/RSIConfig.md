# `RSIConfig`

```ts
type RSIConfig = {
  bandColor?: string | number;
  bandsVisible?: boolean;
  enabled?: boolean;
  extremeFill?: boolean;
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

Source: [types/src/index.ts:513](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L513)

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

Source: [types/src/index.ts:540](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L540)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:542](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L542)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:515](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L515)

Draw the pane. Default false.

---

### extremeFill?

```ts
optional extremeFill?: boolean;
```

Source: [types/src/index.ts:552](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L552)

Shade the stretches where the RSI line sits past a band, fading out at the
rule and deepening toward the end of the scale, so how far a reading went
past the threshold reads at a glance. Default true.

Colored from the theme's `accentBull` (overbought) and `accentBear`
(oversold), the same pair the volume bars and MACD histogram use. Never
reaches full opacity — the line and its rule stay legible through it.

---

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:530](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L530)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:534](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L534)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:532](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L532)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:521](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L521)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:536](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L536)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:523](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L523)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:525](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L525)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:527](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L527)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:538](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L538)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:517](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L517)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:519](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L519)

Overbought band level (0..100). Default 70.
