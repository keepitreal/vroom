# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:213](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L213)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:217](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L217)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:215](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L215)

Window start (inclusive), Unix epoch milliseconds.
