# `MACDConfig`

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Source: [types/src/index.ts:470](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L470)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:471](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L471)

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:473](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L473)

Fast EMA length. Default 12.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:477](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L477)

Signal-line EMA length. Default 9.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:475](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L475)

Slow EMA length (forced > fast). Default 26.
