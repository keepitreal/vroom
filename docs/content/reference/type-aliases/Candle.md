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

Defined in: [types/src/index.ts:8](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L8)

A single OHLCV bar. `candles` is an array of these.

## Properties

### close

```ts
close: number;
```

Defined in: [types/src/index.ts:18](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L18)

Closing price.

***

### high

```ts
high: number;
```

Defined in: [types/src/index.ts:14](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L14)

Highest price during the bar.

***

### low

```ts
low: number;
```

Defined in: [types/src/index.ts:16](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L16)

Lowest price during the bar.

***

### open

```ts
open: number;
```

Defined in: [types/src/index.ts:12](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L12)

Opening price.

***

### timeMs

```ts
timeMs: number;
```

Defined in: [types/src/index.ts:10](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L10)

Bar open time as Unix epoch milliseconds.

***

### volume

```ts
volume: number;
```

Defined in: [types/src/index.ts:20](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L20)

Traded volume during the bar.
