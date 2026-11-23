---
title: "MACDConfig"
description: "MACD indicator config."
---

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

Source: [types/src/index.ts:1420](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1420)

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

Source: [types/src/index.ts:1422](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1422)

Draw the pane. Default false.

***

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:1424](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1424)

Fast moving-average length. Default 12.

***

### histogramDownColor?

```ts
optional histogramDownColor?: string | number;
```

Source: [types/src/index.ts:1463](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1463)

Bars below zero still growing away from it. Defaults to `theme.accentBear`.

***

### histogramDownFadingColor?

```ts
optional histogramDownFadingColor?: string | number;
```

Source: [types/src/index.ts:1468](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1468)

Bars below zero rising back toward it. Defaults to `histogramDownColor` at
half opacity.

***

### histogramUpColor?

```ts
optional histogramUpColor?: string | number;
```

Source: [types/src/index.ts:1456](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1456)

Bars above zero that are still growing away from it. Defaults to
`theme.accentBull`. Set all four histogram colors alike for a flat,
single-color histogram.

***

### histogramUpFadingColor?

```ts
optional histogramUpFadingColor?: string | number;
```

Source: [types/src/index.ts:1461](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1461)

Bars above zero that are falling back toward it, i.e. momentum easing.
Defaults to `histogramUpColor` at half opacity.

***

### histogramVisible?

```ts
optional histogramVisible?: boolean;
```

Source: [types/src/index.ts:1450](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1450)

Draw the histogram bars. Default true.

***

### lineColor?

```ts
optional lineColor?: string | number;
```

Source: [types/src/index.ts:1437](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1437)

MACD line color (hex string or packed ARGB number). Default blue.

***

### lineVisible?

```ts
optional lineVisible?: boolean;
```

Source: [types/src/index.ts:1441](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1441)

Draw the MACD line. Default true.

***

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1439](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1439)

MACD line stroke width in px. Default 1.5.

***

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:1432](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1432)

Averaging used for the fast and slow legs ([MAKind](MAKind.md)). Default 'ema'.

***

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:1428](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1428)

Signal-line length. Default 9.

***

### signalColor?

```ts
optional signalColor?: string | number;
```

Source: [types/src/index.ts:1443](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1443)

Signal line color. Default orange.

***

### signalMaType?

```ts
optional signalMaType?: MAKind;
```

Source: [types/src/index.ts:1434](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1434)

Averaging applied to the MACD series for the signal line. Default 'ema'.

***

### signalVisible?

```ts
optional signalVisible?: boolean;
```

Source: [types/src/index.ts:1447](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1447)

Draw the signal line. Default true.

***

### signalWidth?

```ts
optional signalWidth?: number;
```

Source: [types/src/index.ts:1445](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1445)

Signal line stroke width in px. Default 1.5.

***

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:1426](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1426)

Slow moving-average length (forced > fast). Default 26.

***

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:1430](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1430)

Price source for the fast/slow legs ([MASource](MASource.md)). Default 'close'.

***

### zeroLineColor?

```ts
optional zeroLineColor?: string | number;
```

Source: [types/src/index.ts:1471](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1471)

Zero-reference line color. Default gray.

***

### zeroLineVisible?

```ts
optional zeroLineVisible?: boolean;
```

Source: [types/src/index.ts:1473](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1473)

Draw the zero-reference line. Default true.
