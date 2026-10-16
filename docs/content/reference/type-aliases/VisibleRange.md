# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:250](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L250)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:254](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L254)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:252](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L252)

Window start (inclusive), Unix epoch milliseconds.
