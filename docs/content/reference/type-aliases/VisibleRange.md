# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:153](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L153)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:157](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L157)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:155](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L155)

Window start (inclusive), Unix epoch milliseconds.
