# `PriceLine`

```ts
type PriceLine = {
  axisLabel?: boolean;
  badgeTextColor?: VroomColor;
  closable?: boolean;
  color?: VroomColor;
  draggable?: boolean;
  extendLeft?: boolean;
  id: string;
  lineStyle?: "solid" | "dotted" | "dashed";
  price: number;
  quantity?: string;
  text?: string;
  width?: number;
};
```

Source: [types/src/index.ts:1026](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1026)

A consumer-supplied horizontal status line at a fixed price — the primitive
behind resting limit orders, take-profits, stop-losses and liquidation levels.

Renders as a line across the price pane ending in a label group (a `text` pill
plus an optional solid-filled `quantity` pill and an optional close button),
with a price badge in the y-axis strip.

Interaction is opt-in and callback-gated: the line is only draggable when
`draggable` is set, and the close button only renders when you pass
`onPriceLineClose`. Dragging is a _preview_ — the chart never mutates
the price you gave it, so a move your backend rejects reverts on its own
simply by leaving your `priceLines` state unchanged.

## Properties

### axisLabel?

```ts
optional axisLabel?: boolean;
```

Source: [types/src/index.ts:1067](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1067)

Show the price badge in the y-axis strip. Default true.

---

### badgeTextColor?

```ts
optional badgeTextColor?: VroomColor;
```

Source: [types/src/index.ts:1048](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1048)

Text on this line's filled pieces: the y-axis price badge and the
`quantity` pill. Defaults to `theme.badgeText`. Set it when `color` is
light enough that the shared badge text doesn't read against it.

---

### closable?

```ts
optional closable?: boolean;
```

Source: [types/src/index.ts:1063](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1063)

Show the close button on this line. Defaults to true, but the button only
ever renders if you also pass `onPriceLineClose` — set this to false to opt a
single line out (e.g. a liquidation level the user can't dismiss).

---

### color?

```ts
optional color?: VroomColor;
```

Source: [types/src/index.ts:1042](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1042)

Line, border, body text and close-icon color, and the fill of the
`quantity` pill and the y-axis price badge. Defaults to a soft red.

---

### draggable?

```ts
optional draggable?: boolean;
```

Source: [types/src/index.ts:1057](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1057)

Let the user drag this line vertically to a new price. Default false.
Pair with `onPriceLineDragEnd` to commit the move.

---

### extendLeft?

```ts
optional extendLeft?: boolean;
```

Source: [types/src/index.ts:1065](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1065)

Extend the line to the pane's left edge. Default true.

---

### id

```ts
id: string;
```

Source: [types/src/index.ts:1028](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1028)

Stable unique id, echoed back by every callback.

---

### lineStyle?

```ts
optional lineStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:1052](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1052)

Line style. Default `'dotted'`, matching the current-price indicator.

---

### price

```ts
price: number;
```

Source: [types/src/index.ts:1030](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1030)

Where the line sits on the price scale.

---

### quantity?

```ts
optional quantity?: string;
```

Source: [types/src/index.ts:1037](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1037)

Trailing segment, drawn as a pill filled with `color` so size reads at a
glance (e.g. `'x 5.206'`). Its text is `badgeTextColor`. Omit to hide it.

---

### text?

```ts
optional text?: string;
```

Source: [types/src/index.ts:1032](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1032)

Body label — render whatever you like (e.g. `'Limit Buy @ 13.79'`).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:1050](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1050)

Stroke width in px. Default 1.
