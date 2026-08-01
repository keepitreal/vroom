# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:353](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L353)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:354](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L354)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:356](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L356)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:360](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L360)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:358](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L358)

Slow EMA length (forced > fast). Default 26.
