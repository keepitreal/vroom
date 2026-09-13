# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:837](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L837)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:844](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L844)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:851](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L851)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:846](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L846)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:839](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L839)

Badge radius in px. Default 9.
