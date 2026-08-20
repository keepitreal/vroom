# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:146](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L146)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:150](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L150)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:148](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L148)

Window start (inclusive), Unix epoch milliseconds.
