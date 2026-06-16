# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types.ts:126](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L126)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types.ts:127](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L127)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types.ts:129](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L129)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types.ts:133](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L133)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types.ts:131](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L131)

Slow EMA length (forced > fast). Default 26.
