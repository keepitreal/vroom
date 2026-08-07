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

Source: [types/src/index.ts:439](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L439)

Shared layout/style for every price line, passed via `priceLinesStyle`.

## Properties

### align?

```ts
optional align?: "left" | "center" | "right";
```

Source: [types/src/index.ts:454](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L454)

Where the label group sits horizontally. Default `'right'`.

---

### bodyBackground?

```ts
optional bodyBackground?: VroomColor;
```

Source: [types/src/index.ts:444](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L444)

Translucent fill behind the body and close-button pills, so the label reads
over candles without hiding them. Defaults to a dark translucent grey.

---

### fontSize?

```ts
optional fontSize?: number;
```

Source: [types/src/index.ts:446](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L446)

Label font size in px. Defaults to the axis font size.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:460](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L460)

How much the hovered line or close button brightens, as a channel
multiplier. 1 disables the highlight. Default 1.25. Web only — touch
platforms have no hover state.

---

### inset?

```ts
optional inset?: number;
```

Source: [types/src/index.ts:452](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L452)

How far in from the price axis the label group sits, as a fraction of pane
width (0 = flush against the axis, 0.5 = at the pane's midpoint). Only
applies when `align` is `'right'`. Default 0.
