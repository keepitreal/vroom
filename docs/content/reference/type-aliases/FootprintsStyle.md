# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:991](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L991)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:998](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L998)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1005](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1005)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:1000](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1000)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:993](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L993)

Badge radius in px. Default 9.
