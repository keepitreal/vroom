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

Source: [types/src/index.ts:905](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L905)

Shared layout/style for every price line, passed via `priceLinesStyle`.

## Properties

### align?

```ts
optional align?: "left" | "center" | "right";
```

Source: [types/src/index.ts:920](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L920)

Where the label group sits horizontally. Default `'right'`.

---

### bodyBackground?

```ts
optional bodyBackground?: VroomColor;
```

Source: [types/src/index.ts:910](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L910)

Translucent fill behind the body and close-button pills, so the label reads
over candles without hiding them. Defaults to a dark translucent grey.

---

### fontSize?

```ts
optional fontSize?: number;
```

Source: [types/src/index.ts:912](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L912)

Label font size in px. Defaults to the axis font size.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:926](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L926)

How much the hovered line or close button brightens, as a channel
multiplier. 1 disables the highlight. Default 1.25. Web only — touch
platforms have no hover state.

---

### inset?

```ts
optional inset?: number;
```

Source: [types/src/index.ts:918](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L918)

How far in from the price axis the label group sits, as a fraction of pane
width (0 = flush against the axis, 0.5 = at the pane's midpoint). Only
applies when `align` is `'right'`. Default 0.
