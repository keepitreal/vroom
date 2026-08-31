# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:173](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L173)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:177](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L177)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:175](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L175)

Window start (inclusive), Unix epoch milliseconds.
