# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:200](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L200)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:204](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L204)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:202](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L202)

Window start (inclusive), Unix epoch milliseconds.
