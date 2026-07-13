# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:243](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L243)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:244](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L244)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:246](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L246)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:250](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L250)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:248](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L248)

Slow EMA length (forced > fast). Default 26.
