---
title: "PlotRect"
description: "The chart's plot area in logical px relative to the chart element's top-left: the candles and everything drawn over them, with the price and time axis strips excluded."
---

```ts
type PlotRect = {
  bottom: number;
  left: number;
  right: number;
  top: number;
};
```

Source: [types/src/index.ts:1200](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1200)

The chart's plot area in logical px relative to the chart element's top-left:
the candles and everything drawn over them, with the price and time axis
strips excluded.

This is the rect to test a floating UI against, and it is deliberately *not*
the element's own box — the element includes the axis strips, so measuring it
overstates the room beside anything near an edge.

## Properties

### bottom

```ts
bottom: number;
```

Source: [types/src/index.ts:1204](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1204)

***

### left

```ts
left: number;
```

Source: [types/src/index.ts:1201](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1201)

***

### right

```ts
right: number;
```

Source: [types/src/index.ts:1203](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1203)

***

### top

```ts
top: number;
```

Source: [types/src/index.ts:1202](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1202)
