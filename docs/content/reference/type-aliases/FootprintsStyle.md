# `FootprintsStyle`

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:1492](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1492)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### ~~gap?~~

```ts
optional gap?: number;
```

Source: [types/src/index.ts:1499](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1499)

#### Deprecated

Unused: a candle with both a buy and a sell now draws one
combined badge, so there is no stack to space out.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1509](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1509)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

---

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:1504](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1504)

Gap between the candle's high and the badge, in px, in candle mode. Default 8. In line mode the badge sits on the line instead.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:1494](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1494)

Badge radius in px. Default 9.
