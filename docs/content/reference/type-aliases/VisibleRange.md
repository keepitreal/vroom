# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:224](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L224)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:228](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L228)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:226](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L226)

Window start (inclusive), Unix epoch milliseconds.
