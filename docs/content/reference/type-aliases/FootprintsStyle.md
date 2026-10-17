# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:1058](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1058)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:1065](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1065)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1072](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1072)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:1067](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1067)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:1060](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1060)

Badge radius in px. Default 9.
