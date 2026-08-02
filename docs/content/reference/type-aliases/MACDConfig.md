# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:390](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L390)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:391](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L391)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:393](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L393)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:397](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L397)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:395](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L395)

Slow EMA length (forced > fast). Default 26.
