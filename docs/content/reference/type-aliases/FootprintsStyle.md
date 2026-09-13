# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:943](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L943)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:950](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L950)

Vertical gap between the two stacked badges on a candle that has both a buy
and a sell. Default 4 — wide enough that each stays independently hoverable.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:957](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L957)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:952](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L952)

Gap between the candle's high and the first badge, in px. Default 8.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:945](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L945)

Badge radius in px. Default 9.
