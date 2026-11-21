---
title: "VisibleRange"
description: "A time window over the candle data, as Unix epoch milliseconds."
---

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Source: [types/src/index.ts:285](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L285)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Source: [types/src/index.ts:289](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L289)

Window end (inclusive), Unix epoch milliseconds.

***

### startMs

```ts
startMs: number;
```

Source: [types/src/index.ts:287](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L287)

Window start (inclusive), Unix epoch milliseconds.
