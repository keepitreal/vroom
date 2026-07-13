# Type Alias: RSIConfig

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

Defined in: [types/src/index.ts:140](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L140)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:141](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L141)

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Defined in: [types/src/index.ts:147](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L147)

Oversold band level (0..100). Default 30.

***

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Defined in: [types/src/index.ts:149](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L149)

Show the RSI-based moving-average trendline. Default true.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Defined in: [types/src/index.ts:151](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L151)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### period?

```ts
optional period?: number;
```

Defined in: [types/src/index.ts:143](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L143)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Defined in: [types/src/index.ts:145](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L145)

Overbought band level (0..100). Default 70.
