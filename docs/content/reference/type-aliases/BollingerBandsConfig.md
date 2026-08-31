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

Source: [types/src/index.ts:489](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L489)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:491](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L491)

Draw the bands. Default false.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:519](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L519)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:517](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L517)

Draw the translucent fill between the bands. Default true.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:513](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L513)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:515](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L515)

Lower band stroke width in px. Default 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:503](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L503)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:509](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L509)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:511](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L511)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:493](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L493)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:497](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L497)

Price source ([MASource](MASource.md)). Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:495](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L495)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:505](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L505)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:507](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L507)

Upper band stroke width in px. Default 1.
