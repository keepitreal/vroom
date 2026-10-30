# `PriceLinesStyle`

```ts
type PriceLinesStyle = {
  align?: "left" | "center" | "right";
  bodyBackground?: VroomColor;
  cornerRadius?: number;
  fontSize?: number;
  hoverBoost?: number;
  inset?: number;
};
```

Source: [types/src/index.ts:1071](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1071)

Shared layout/style for every price line, passed via `priceLinesStyle`.

## Properties

### align?

```ts
optional align?: "left" | "center" | "right";
```

Source: [types/src/index.ts:1090](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1090)

Where the label group sits horizontally. Default `'right'`.

---

### bodyBackground?

```ts
optional bodyBackground?: VroomColor;
```

Source: [types/src/index.ts:1076](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1076)

Translucent fill behind the body and close-button pills, so the label reads
over candles without hiding them. Defaults to a dark translucent grey.

---

### cornerRadius?

```ts
optional cornerRadius?: number;
```

Source: [types/src/index.ts:1101](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1101)

Corner radius of the label pills in CSS px, clamped to 0–6. Omit for the
current 6px radius. 0 draws square corners.

---

### fontSize?

```ts
optional fontSize?: number;
```

Source: [types/src/index.ts:1082](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1082)

Label font size in integer CSS px, clamped to 10–14. Omit to follow
`theme.badgeFontSize`, which itself defaults to `theme.axisFontSize` (11px
by default, also clamped into that range).

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1096](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1096)

How much the hovered line or close button brightens, as a channel
multiplier. 1 disables the highlight. Default 1.25. Web only — touch
platforms have no hover state.

---

### inset?

```ts
optional inset?: number;
```

Source: [types/src/index.ts:1088](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1088)

How far in from the price axis the label group sits, as a fraction of pane
width (0 = flush against the axis, 0.5 = at the pane's midpoint). Only
applies when `align` is `'right'`. Default 0.
