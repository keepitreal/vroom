# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:975](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L975)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:982](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L982)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:989](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L989)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:984](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L984)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:977](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L977)

Badge radius in px. Default 9.
