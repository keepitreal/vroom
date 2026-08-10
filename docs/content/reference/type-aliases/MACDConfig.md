# `MACDConfig`

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Source: [types/src/index.ts:511](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L511)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:512](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L512)

---

### fast?

```ts
optional fast?: number;
```

Source: [types/src/index.ts:514](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L514)

Fast EMA length. Default 12.

---

### signal?

```ts
optional signal?: number;
```

Source: [types/src/index.ts:518](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L518)

Signal-line EMA length. Default 9.

---

### slow?

```ts
optional slow?: number;
```

Source: [types/src/index.ts:516](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L516)

Slow EMA length (forced > fast). Default 26.
