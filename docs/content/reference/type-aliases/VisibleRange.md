# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:180](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L180)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:184](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L184)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:182](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L182)

Window start (inclusive), Unix epoch milliseconds.
