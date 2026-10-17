# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:251](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L251)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:255](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L255)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:253](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L253)

Window start (inclusive), Unix epoch milliseconds.
