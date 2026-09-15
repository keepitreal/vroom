# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:186](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L186)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:190](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L190)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:188](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L188)

Window start (inclusive), Unix epoch milliseconds.
