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

Source: [types/src/index.ts:612](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L612)

Bollinger Bands overlay config. A basis moving average of `source` over
`period`, banded at ± `stdDev` × population standard deviation of the same
window, drawn as three lines on the price pane with an optional translucent
fill between the bands. No pane is reserved.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:614](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L614)

Draw the bands. Default false.

---

### fillOpacity?

```ts
optional fillOpacity?: number;
```

Source: [types/src/index.ts:642](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L642)

Fill opacity 0..1, applied to the upper band color. Default 0.1.

---

### fillVisible?

```ts
optional fillVisible?: boolean;
```

Source: [types/src/index.ts:640](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L640)

Draw the translucent fill between the bands. Default true.

---

### lowerColor?

```ts
optional lowerColor?: string | number;
```

Source: [types/src/index.ts:636](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L636)

Lower band color. Default blue.

---

### lowerWidth?

```ts
optional lowerWidth?: number;
```

Source: [types/src/index.ts:638](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L638)

Lower band stroke width in px. Default 1.

---

### maType?

```ts
optional maType?: MAKind;
```

Source: [types/src/index.ts:626](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L626)

Averaging for the basis (middle) line ([MAKind](MAKind.md)). Default 'sma'. The
stdev always uses the window's arithmetic mean, even with an EMA basis
(the standard semantics).

---

### middleColor?

```ts
optional middleColor?: string | number;
```

Source: [types/src/index.ts:632](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L632)

Basis (middle) line color. Default orange.

---

### middleWidth?

```ts
optional middleWidth?: number;
```

Source: [types/src/index.ts:634](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L634)

Basis line stroke width in px. Default 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:616](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L616)

Lookback in candles. Default 20, clamped to >= 1.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:620](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L620)

Price source ([MASource](MASource.md)). Default 'close'.

---

### stdDev?

```ts
optional stdDev?: number;
```

Source: [types/src/index.ts:618](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L618)

Standard-deviation multiplier. Default 2.

---

### upperColor?

```ts
optional upperColor?: string | number;
```

Source: [types/src/index.ts:628](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L628)

Upper band color (hex string or packed ARGB number). Default blue.

---

### upperWidth?

```ts
optional upperWidth?: number;
```

Source: [types/src/index.ts:630](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L630)

Upper band stroke width in px. Default 1.
