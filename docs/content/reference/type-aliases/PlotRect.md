# `PlotRect`

```ts
type PlotRect = {
  bottom: number;
  left: number;
  right: number;
  top: number;
};
```

Source: [types/src/index.ts:995](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L995)

The chart's plot area in logical px relative to the chart element's top-left:
the candles and everything drawn over them, with the price and time axis
strips excluded.

This is the rect to test a floating UI against, and it is deliberately _not_
the element's own box — the element includes the axis strips, so measuring it
overstates the room beside anything near an edge.

## Properties

### bottom

```ts
bottom: number;
```

Source: [types/src/index.ts:999](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L999)

---

### left

```ts
left: number;
```

Source: [types/src/index.ts:996](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L996)

---

### right

```ts
right: number;
```

Source: [types/src/index.ts:998](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L998)

---

### top

```ts
top: number;
```

Source: [types/src/index.ts:997](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L997)
