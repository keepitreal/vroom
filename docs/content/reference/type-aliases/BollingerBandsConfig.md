# `BollingerBandsConfig`

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

Source: [types/src/index.ts:391](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L391)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:393](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L393)

Draw the bands. Default false.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:421](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L421)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:419](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L419)

Draw the translucent fill between the bands. Default true.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:415](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L415)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:417](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L417)

Lower band stroke width in px. Default 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:405](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L405)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:411](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L411)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:413](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L413)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:395](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L395)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:399](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L399)

Price source ([MASource](MASource.md)). Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:397](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L397)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:407](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L407)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:409](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L409)

Upper band stroke width in px. Default 1.
