# `RSIConfig`

```ts
type RSIConfig = {
  enabled?: boolean;
  lowerBand?: number;
  maEnabled?: boolean;
  maPeriod?: number;
  period?: number;
  upperBand?: number;
};
```

Source: [types/src/index.ts:262](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L262)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:263](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L263)

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:269](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L269)

Oversold band level (0..100). Default 30.

---

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Source: [types/src/index.ts:271](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L271)

Show the RSI-based moving-average trendline. Default true.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:273](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L273)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:265](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L265)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:267](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L267)

Overbought band level (0..100). Default 70.
