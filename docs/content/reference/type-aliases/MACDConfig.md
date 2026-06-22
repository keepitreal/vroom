# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:130](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L130)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:131](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L131)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:133](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L133)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:137](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L137)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:135](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L135)

Slow EMA length (forced > fast). Default 26.
