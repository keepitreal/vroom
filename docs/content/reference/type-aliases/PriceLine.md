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

Source: [types/src/index.ts:905](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L905)

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

Source: [types/src/index.ts:937](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L937)

Show the price badge in the y-axis strip. Default true.

---

### closable?

```ts
optional closable?: boolean;
```

Source: [types/src/index.ts:933](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L933)

Show the close button on this line. Defaults to true, but the button only
ever renders if you also pass `onPriceLineClose` — set this to false to opt a
single line out (e.g. a liquidation level the user can't dismiss).

---

### color?

```ts
optional color?: VroomColor;
```

Source: [types/src/index.ts:918](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L918)

Line, border, body text and close-icon color. Defaults to a soft red.

---

### draggable?

```ts
optional draggable?: boolean;
```

Source: [types/src/index.ts:927](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L927)

Let the user drag this line vertically to a new price. Default false.
Pair with `onPriceLineDragEnd` to commit the move.

---

### extendLeft?

```ts
optional extendLeft?: boolean;
```

Source: [types/src/index.ts:935](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L935)

Extend the line to the pane's left edge. Default true.

---

### id

```ts
id: string;
```

Source: [types/src/index.ts:907](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L907)

Stable unique id, echoed back by every callback.

---

### lineStyle?

```ts
optional lineStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:922](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L922)

Line style. Default `'dotted'`, matching the current-price indicator.

---

### price

```ts
price: number;
```

Source: [types/src/index.ts:909](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L909)

Where the line sits on the price scale.

---

### quantity?

```ts
optional quantity?: string;
```

Source: [types/src/index.ts:916](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L916)

Trailing segment, drawn as a solid-filled pill with white text so size reads
at a glance (e.g. `'x 5.206'`). Omit to hide it.

---

### text?

```ts
optional text?: string;
```

Source: [types/src/index.ts:911](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L911)

Body label — render whatever you like (e.g. `'Limit Buy @ 13.79'`).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:920](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L920)

Stroke width in px. Default 1.
