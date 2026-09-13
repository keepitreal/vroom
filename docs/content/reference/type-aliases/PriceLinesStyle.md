# `PriceLinesStyle`

```ts
type PriceLinesStyle = {
  align?: "left" | "center" | "right";
  bodyBackground?: VroomColor;
  fontSize?: number;
  hoverBoost?: number;
  inset?: number;
};
```

Source: [types/src/index.ts:860](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L860)

Shared layout/style for every price line, passed via `priceLinesStyle`.

## Properties

### align?

```ts
optional align?: "left" | "center" | "right";
```

Source: [types/src/index.ts:875](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L875)

Where the label group sits horizontally. Default `'right'`.

---

### bodyBackground?

```ts
optional bodyBackground?: VroomColor;
```

Source: [types/src/index.ts:865](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L865)

Translucent fill behind the body and close-button pills, so the label reads
over candles without hiding them. Defaults to a dark translucent grey.

---

### fontSize?

```ts
optional fontSize?: number;
```

Source: [types/src/index.ts:867](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L867)

Label font size in px. Defaults to the axis font size.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:881](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L881)

How much the hovered line or close button brightens, as a channel
multiplier. 1 disables the highlight. Default 1.25. Web only — touch
platforms have no hover state.

---

### inset?

```ts
optional inset?: number;
```

Source: [types/src/index.ts:873](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L873)

How far in from the price axis the label group sits, as a fraction of pane
width (0 = flush against the axis, 0.5 = at the pane's midpoint). Only
applies when `align` is `'right'`. Default 0.
