# `PlotRect`

```ts
type PlotRect = {
  bottom: number;
  left: number;
  right: number;
  top: number;
};
```

Source: [types/src/index.ts:1046](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1046)

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

Source: [types/src/index.ts:1050](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1050)

---

### left

```ts
left: number;
```

Source: [types/src/index.ts:1047](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1047)

---

### right

```ts
right: number;
```

Source: [types/src/index.ts:1049](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1049)

---

### top

```ts
top: number;
```

Source: [types/src/index.ts:1048](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1048)
