# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:119](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L119)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:123](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L123)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:121](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L121)

Window start (inclusive), Unix epoch milliseconds.
