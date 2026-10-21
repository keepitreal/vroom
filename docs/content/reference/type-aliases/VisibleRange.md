# `VisibleRange`

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:260](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L260)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:264](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L264)

Window end (inclusive), Unix epoch milliseconds.

---

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:262](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L262)

Window start (inclusive), Unix epoch milliseconds.
