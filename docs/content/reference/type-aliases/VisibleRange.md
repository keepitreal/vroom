# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:185](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L185)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:189](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L189)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:187](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L187)

Window start (inclusive), Unix epoch milliseconds.
