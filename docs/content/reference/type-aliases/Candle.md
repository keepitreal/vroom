# Type Alias: Candle

```ts
type Candle = {
  close: number;
  high: number;
  low: number;
  open: number;
  timeMs: number;
  volume: number;
};
```

Defined in: [types.ts:4](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L4)

A single OHLCV bar. `candles` is an array of these.

## Properties

### close

```ts
close: number;
```

Defined in: [types.ts:14](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L14)

Closing price.

***

### high

```ts
high: number;
```

Defined in: [types.ts:10](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L10)

Highest price during the bar.

***

### low

```ts
low: number;
```

Defined in: [types.ts:12](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L12)

Lowest price during the bar.

***

### open

```ts
open: number;
```

Defined in: [types.ts:8](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L8)

Opening price.

***

### timeMs

```ts
timeMs: number;
```

Defined in: [types.ts:6](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L6)

Bar open time as Unix epoch milliseconds.

***

### volume

```ts
volume: number;
```

Defined in: [types.ts:16](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L16)

Traded volume during the bar.
