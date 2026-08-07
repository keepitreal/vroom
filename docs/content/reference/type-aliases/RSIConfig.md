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

Source: [types/src/index.ts:250](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L250)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:251](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L251)

---

### lowerBand?

```ts
optional lowerBand?: number;
```

Source: [types/src/index.ts:257](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L257)

Oversold band level (0..100). Default 30.

---

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Source: [types/src/index.ts:259](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L259)

Show the RSI-based moving-average trendline. Default true.

---

### maPeriod?

```ts
optional maPeriod?: number;
```

Source: [types/src/index.ts:261](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L261)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

---

### period?

```ts
optional period?: number;
```

Source: [types/src/index.ts:253](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L253)

Lookback period in candle counts. Default 14, clamped to >= 2.

---

### upperBand?

```ts
optional upperBand?: number;
```

Source: [types/src/index.ts:255](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L255)

Overbought band level (0..100). Default 70.
