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

Source: [types/src/index.ts:649](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L649)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:651](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L651)

Draw the bands. Default false.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:679](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L679)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:677](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L677)

Draw the translucent fill between the bands. Default true.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:673](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L673)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:675](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L675)

Lower band stroke width in px. Default 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:663](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L663)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:669](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L669)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:671](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L671)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:653](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L653)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:657](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L657)

Price source ([MASource](MASource.md)). Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:655](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L655)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:665](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L665)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:667](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L667)

Upper band stroke width in px. Default 1.
