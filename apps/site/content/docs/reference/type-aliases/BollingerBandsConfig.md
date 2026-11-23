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

Source: [types/src/index.ts:750](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L750)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:752](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L752)

Draw the bands. Default false.

***

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:780](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L780)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

***

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:778](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L778)

Draw the translucent fill between the bands. Default true.

***

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:774](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L774)

Lower band color. Default blue.

***

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:776](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L776)

Lower band stroke width in px. Default 1.

***

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:764](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L764)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

***

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:770](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L770)

Basis (middle) line color. Default orange.

***

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:772](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L772)

Basis line stroke width in px. Default 1.

***

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:754](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L754)

Lookback in candles. Default 20, clamped to >= 1.

***

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:758](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L758)

Price source ([MASource](MASource.md)). Default 'close'.

***

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:756](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L756)

Standard-deviation multiplier. Default 2.

***

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:766](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L766)

Upper band color (hex string or packed ARGB number). Default blue.

***

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:768](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L768)

Upper band stroke width in px. Default 1.
