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

Defined in: [types/src/index.ts:223](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L223)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:224](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L224)

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Defined in: [types/src/index.ts:230](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L230)

Oversold band level (0..100). Default 30.

***

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Defined in: [types/src/index.ts:232](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L232)

Show the RSI-based moving-average trendline. Default true.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Defined in: [types/src/index.ts:234](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L234)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### period?

```ts
optional period?: number;
```

Defined in: [types/src/index.ts:226](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L226)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Defined in: [types/src/index.ts:228](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L228)

Overbought band level (0..100). Default 70.
