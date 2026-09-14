# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:952](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L952)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:959](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L959)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:966](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L966)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:961](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L961)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:954](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L954)

Badge radius in px. Default 9.
