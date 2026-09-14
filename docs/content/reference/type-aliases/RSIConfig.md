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

Source: [types/src/index.ts:483](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L483)

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

Source: [types/src/index.ts:510](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L510)

Color of both dashed band rules. Default gray.

---

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:512](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L512)

Draw the overbought/oversold rules. Default true.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:485](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L485)

Draw the pane. Default false.

---

### extremeFill?

```ts
optional extremeFill?: boolean;
```

Source: [types/src/index.ts:522](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L522)

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

Source: [types/src/index.ts:500](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L500)

RSI line color (hex string or packed ARGB number). Default violet.

---

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:504](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L504)

Draw the RSI line. Default true.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:502](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L502)

RSI line stroke width in px. Default 1.5.

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:491](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L491)

Oversold band level (0..100). Default 30.

---

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:506](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L506)

Trendline color. Default amber.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:493](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L493)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:495](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L495)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

---

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:497](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L497)

Draw the moving-average trendline. Default true.

---

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:508](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L508)

Trendline stroke width in px. Default 1.5.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:487](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L487)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:489](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L489)

Overbought band level (0..100). Default 70.
