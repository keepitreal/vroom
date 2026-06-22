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

Defined in: [types/src/index.ts:74](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L74)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:75](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L75)

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Defined in: [types/src/index.ts:81](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L81)

Oversold band level (0..100). Default 30.

***

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Defined in: [types/src/index.ts:83](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L83)

Show the RSI-based moving-average trendline. Default true.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Defined in: [types/src/index.ts:85](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L85)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### period?

```ts
optional period?: number;
```

Defined in: [types/src/index.ts:77](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L77)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Defined in: [types/src/index.ts:79](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L79)

Overbought band level (0..100). Default 70.
