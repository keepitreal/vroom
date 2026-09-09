# `PriceLine`

```ts
type PriceLine = {
  axisLabel?: boolean;
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

Source: [types/src/index.ts:660](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L660)

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

Source: [types/src/index.ts:692](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L692)

Show the price badge in the y-axis strip. Default true.

---

### closable?

```ts
optional closable?: boolean;
```

Source: [types/src/index.ts:688](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L688)

Show the close button on this line. Defaults to true, but the button only
ever renders if you also pass `onPriceLineClose` — set this to false to opt a
single line out (e.g. a liquidation level the user can't dismiss).

---

### color?

```ts
optional color?: VroomColor;
```

Source: [types/src/index.ts:673](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L673)

Line, border, body text and close-icon color. Defaults to a soft red.

---

### draggable?

```ts
optional draggable?: boolean;
```

Source: [types/src/index.ts:682](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L682)

Let the user drag this line vertically to a new price. Default false.
Pair with `onPriceLineDragEnd` to commit the move.

---

### extendLeft?

```ts
optional extendLeft?: boolean;
```

Source: [types/src/index.ts:690](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L690)

Extend the line to the pane's left edge. Default true.

---

### id

```ts
id: string;
```

Source: [types/src/index.ts:662](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L662)

Stable unique id, echoed back by every callback.

---

### lineStyle?

```ts
optional lineStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:677](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L677)

Line style. Default `'dotted'`, matching the current-price indicator.

---

### price

```ts
price: number;
```

Source: [types/src/index.ts:664](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L664)

Where the line sits on the price scale.

---

### quantity?

```ts
optional quantity?: string;
```

Source: [types/src/index.ts:671](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L671)

Trailing segment, drawn as a solid-filled pill with white text so size reads
at a glance (e.g. `'x 5.206'`). Omit to hide it.

---

### text?

```ts
optional text?: string;
```

Source: [types/src/index.ts:666](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L666)

Body label — render whatever you like (e.g. `'Limit Buy @ 13.79'`).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:675](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L675)

Stroke width in px. Default 1.
