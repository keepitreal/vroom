# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:464](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L464)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:465](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L465)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:467](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L467)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:471](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L471)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:469](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L469)

Slow EMA length (forced > fast). Default 26.
