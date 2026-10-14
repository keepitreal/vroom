# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:232](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L232)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:236](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L236)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:234](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L234)

Window start (inclusive), Unix epoch milliseconds.
