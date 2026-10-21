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

Source: [types/src/index.ts:579](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L579)

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

Source: [types/src/index.ts:606](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L606)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:608](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L608)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:581](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L581)

Draw the pane. Default false.

---

### extremeFill?

```ts
optional extremeFill?: boolean;
```

Source: [types/src/index.ts:618](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L618)

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

Source: [types/src/index.ts:596](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L596)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:600](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L600)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:598](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L598)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:587](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L587)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:602](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L602)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:589](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L589)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:591](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L591)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:593](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L593)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:604](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L604)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:583](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L583)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:585](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L585)

Overbought band level (0..100). Default 70.
