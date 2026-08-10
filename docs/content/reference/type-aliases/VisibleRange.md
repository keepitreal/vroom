# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:114](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L114)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:118](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L118)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:116](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L116)

Window start (inclusive), Unix epoch milliseconds.
