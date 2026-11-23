---
title: "RSIConfig"
description: "RSI indicator config."
---

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

Source: [types/src/index.ts:666](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L666)

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

Source: [types/src/index.ts:693](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L693)

Color of both dashed band rules. Default gray.

***

### bandsVisible?

```ts
optional bandsVisible?: boolean;
```

Source: [types/src/index.ts:695](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L695)

Draw the overbought/oversold rules. Default true.

***

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:668](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L668)

Draw the pane. Default false.

***

### extremeFill?

```ts
optional extremeFill?: boolean;
```

Source: [types/src/index.ts:705](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L705)

Shade the stretches where the RSI line sits past a band, fading out at the
rule and deepening toward the end of the scale, so how far a reading went
past the threshold reads at a glance. Default true.

Colored from the theme's `accentBull` (overbought) and `accentBear`
(oversold), the same pair the volume bars and MACD histogram use. Never
reaches full opacity — the line and its rule stay legible through it.

***

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:683](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L683)

RSI line color (hex string or packed ARGB number). Default violet.

***

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:687](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L687)

Draw the RSI line. Default true.

***

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:685](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L685)

RSI line stroke width in px. Default 1.5.

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:674](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L674)

Oversold band level (0..100). Default 30.

***

### maColor?

```ts
optional maColor?: string | number;
```

Source: [types/src/index.ts:689](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L689)

Trendline color. Default amber.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:676](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L676)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:678](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L678)

Averaging used for the trendline ([MAKind](MAKind.md)). Default 'sma'.

***

### maVisible?

```ts
optional maVisible?: boolean;
```

Source: [types/src/index.ts:680](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L680)

Draw the moving-average trendline. Default true.

***

### maWidth?

```ts
optional maWidth?: number;
```

Source: [types/src/index.ts:691](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L691)

Trendline stroke width in px. Default 1.5.

***

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:670](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L670)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:672](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L672)

Overbought band level (0..100). Default 70.
