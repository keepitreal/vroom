# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:194](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L194)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:198](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L198)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:196](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L196)

Window start (inclusive), Unix epoch milliseconds.
