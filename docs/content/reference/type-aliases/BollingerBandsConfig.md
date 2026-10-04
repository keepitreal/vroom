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

Source: [types/src/index.ts:593](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L593)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:595](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L595)

Draw the bands. Default false.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:623](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L623)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:621](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L621)

Draw the translucent fill between the bands. Default true.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:617](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L617)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:619](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L619)

Lower band stroke width in px. Default 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:607](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L607)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:613](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L613)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:615](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L615)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:597](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L597)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:601](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L601)

Price source ([MASource](MASource.md)). Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:599](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L599)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:609](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L609)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:611](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L611)

Upper band stroke width in px. Default 1.
