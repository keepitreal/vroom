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

Defined in: [types.ts:70](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L70)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types.ts:71](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L71)

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Defined in: [types.ts:77](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L77)

Oversold band level (0..100). Default 30.

***

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Defined in: [types.ts:79](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L79)

Show the RSI-based moving-average trendline. Default true.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Defined in: [types.ts:81](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L81)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### period?

```ts
optional period?: number;
```

Defined in: [types.ts:73](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L73)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Defined in: [types.ts:75](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L75)

Overbought band level (0..100). Default 70.
