# `MACDConfig`

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Source: [types/src/index.ts:476](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L476)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:477](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L477)

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:479](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L479)

Fast EMA length. Default 12.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:483](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L483)

Signal-line EMA length. Default 9.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:481](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L481)

Slow EMA length (forced > fast). Default 26.
