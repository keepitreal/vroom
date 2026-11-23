---
title: "BollingerBandsConfig"
description: "Bollinger Bands overlay config."
---

```ts
type BollingerBandsConfig = {
  enabled?: boolean;
  fillOpacity?: number;
  fillVisible?: boolean;
  lowerColor?: string | number;
  lowerWidth?: number;
  maType?: MAKind;
  middleColor?: string | number;
  middleWidth?: number;
  period?: number;
  source?: MASource;
  stdDev?: number;
  upperColor?: string | number;
  upperWidth?: number;
};
```

Source: [types/src/index.ts:751](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L751)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:753](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L753)

Draw the bands. Default false.

***

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:781](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L781)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

***

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:779](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L779)

Draw the translucent fill between the bands. Default true.

***

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:775](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L775)

Lower band color. Default blue.

***

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:777](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L777)

Lower band stroke width in px. Default 1.

***

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:765](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L765)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

***

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:771](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L771)

Basis (middle) line color. Default orange.

***

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:773](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L773)

Basis line stroke width in px. Default 1.

***

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:755](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L755)

Lookback in candles. Default 20, clamped to >= 1.

***

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:759](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L759)

Price source ([MASource](MASource.md)). Default 'close'.

***

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:757](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L757)

Standard-deviation multiplier. Default 2.

***

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:767](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L767)

Upper band color (hex string or packed ARGB number). Default blue.

***

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:769](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L769)

Upper band stroke width in px. Default 1.
