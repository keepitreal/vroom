# Type Alias: PriceLine

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

Defined in: [types/src/index.ts:403](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L403)

A consumer-supplied horizontal status line at a fixed price — the primitive
behind resting limit orders, take-profits, stop-losses and liquidation levels.

Renders as a line across the price pane ending in a label group (a `text` pill
plus an optional solid-filled `quantity` pill and an optional close button),
with a price badge in the y-axis strip.

Interaction is opt-in and callback-gated, mirroring TradingView: the line is
only draggable when `draggable` is set, and the close button only renders when
you pass `onPriceLineClose`. Dragging is a *preview* — the chart never mutates
the price you gave it, so a move your backend rejects reverts on its own
simply by leaving your `priceLines` state unchanged.

## Properties

### axisLabel?

```ts
optional axisLabel?: boolean;
```

Defined in: [types/src/index.ts:435](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L435)

Show the price badge in the y-axis strip. Default true.

***

### closable?

```ts
optional closable?: boolean;
```

Defined in: [types/src/index.ts:431](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L431)

Show the close button on this line. Defaults to true, but the button only
ever renders if you also pass `onPriceLineClose` — set this to false to opt a
single line out (e.g. a liquidation level the user can't dismiss).

***

### color?

```ts
optional color?: VroomColor;
```

Defined in: [types/src/index.ts:416](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L416)

Line, border, body text and close-icon color. Defaults to a soft red.

***

### draggable?

```ts
optional draggable?: boolean;
```

Defined in: [types/src/index.ts:425](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L425)

Let the user drag this line vertically to a new price. Default false.
Pair with `onPriceLineDragEnd` to commit the move.

***

### extendLeft?

```ts
optional extendLeft?: boolean;
```

Defined in: [types/src/index.ts:433](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L433)

Extend the line to the pane's left edge. Default true.

***

### id

```ts
id: string;
```

Defined in: [types/src/index.ts:405](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L405)

Stable unique id, echoed back by every callback.

***

### lineStyle?

```ts
optional lineStyle?: "solid" | "dotted" | "dashed";
```

Defined in: [types/src/index.ts:420](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L420)

Line style. Default `'dotted'`, matching the current-price indicator.

***

### price

```ts
price: number;
```

Defined in: [types/src/index.ts:407](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L407)

Where the line sits on the price scale.

***

### quantity?

```ts
optional quantity?: string;
```

Defined in: [types/src/index.ts:414](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L414)

Trailing segment, drawn as a solid-filled pill with white text so size reads
at a glance (e.g. `'x 5.206'`). Omit to hide it.

***

### text?

```ts
optional text?: string;
```

Defined in: [types/src/index.ts:409](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L409)

Body label — render whatever you like (e.g. `'Limit Buy @ 13.79'`).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:418](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L418)

Stroke width in px. Default 1.
