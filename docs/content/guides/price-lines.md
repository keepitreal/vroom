# Price lines

Price lines are horizontal status lines you place at specific prices — resting
limit orders, take-profits, stop-losses, liquidation levels, alerts. Each one
draws a styled rule across the price pane, a label group at its right end, and a
price badge in the y-axis strip. They can be dragged to a new price and closed
with a button, which is what makes them usable as live order controls rather than
static annotations.

They work on **both web and React Native** (the renderer lives in the C++ core).

## A minimal line

```tsx
import { VroomChart, type PriceLine } from "@vroomchart/react";

const lines: PriceLine[] = [
  { id: "tp", price: 68420.5, text: "Take Profit", color: "#26a69a" },
];

<VroomChart candles={candles} priceLines={lines} />;
```

That renders a dotted teal line with a `Take Profit` pill and a `68420.50` badge
on the axis. Everything beyond `id` and `price` is optional.

## Anatomy

```
 extendLeft                        label group                   axis
|<-------------- line ----------->|                            |      |
                        ..........[ text ][ quantity ][ × ]----[ badge ]
```

- **`text`** — the body pill: a translucent label with your caption.
- **`quantity`** — a second, solid-filled pill for size or scope (`'0.75'`,
  `'Full position'`). Omit it and the segment disappears.
- **The close button** — a red-on-translucent `×`. See
  [Closing a line](#closing-a-line).
- **The axis badge** — the price, formatted, painted over any axis label it
  overlaps. Turn it off per line with `axisLabel: false`.

A line whose price scrolls outside the visible price range is **hidden**, not
clamped to the pane edge, so an off-screen order can't be mistaken for a nearby
one.

## Dragging

Dragging is **opt in per line** — set `draggable: true`. On the web the cursor
becomes `ns-resize` over a draggable line; on touch, press and drag it directly.

`priceLines` is a **controlled** prop, and that shapes how dragging works: the
chart paints a live *preview* at the pointer and reports the price, but never
mutates the line. It moves for real only when you pass a new array. Which means a
move your backend rejects needs no rollback — leave your state alone and the line
snaps back on its own.

```tsx
function Chart({ candles }) {
  const [lines, setLines] = useState<PriceLine[]>([
    { id: "buy-1", price: 67000, text: "Limit Buy", draggable: true },
  ]);

  return (
    <VroomChart
      candles={candles}
      priceLines={lines}
      // Fires continuously during the drag — good for a live order ticket.
      onPriceLineDrag={(id, price) => setTicketPrice(price)}
      // Fires once on release. Submit here, then commit on success.
      onPriceLineDragEnd={async (id, price) => {
        const ok = await api.repriceOrder(id, price);
        if (ok) {
          setLines((prev) =>
            prev.map((l) => (l.id === id ? { ...l, price } : l)),
          );
        }
        // On failure: do nothing. The line reverts to its committed price.
      }}
    />
  );
}
```

While a drag is in progress a faint ghost marks the starting price, so the user
can see how far they've moved. On the web, **Escape cancels** the drag:
`onPriceLineDragEnd` never fires and the line returns to where it started.

Prices are not snapped to anything — you get the exact price under the pointer.
Round it yourself if your venue has a tick size.

## Closing a line

The close button is **callback-gated**: it renders only if you pass
`onPriceLineClose`. If there's nothing for the button to do, there's no button.

```tsx
<VroomChart
  candles={candles}
  priceLines={lines}
  onPriceLineClose={async (id) => {
    await api.cancelOrder(id);
    setLines((prev) => prev.filter((l) => l.id !== id));
  }}
/>
```

To keep the handler but exclude one line — a liquidation level the user
shouldn't be able to dismiss — set `closable: false` on it.

Like dragging, closing is a request, not an action: the line disappears when you
remove it from the array, so a failed cancellation leaves it on the chart.

## Styling

Per-line styling covers `color` (the line, its border, the body text and the
close icon), `width`, and `lineStyle` (`'solid'`, `'dotted'` — the default — or
`'dashed'`). Layout is shared across all lines via `priceLinesStyle`:

```tsx
<VroomChart
  candles={candles}
  priceLines={lines}
  priceLinesStyle={{
    // Where the label group sits. 'right' (default) parks it near the axis.
    align: "right",
    // Push it further left, as a fraction of pane width. 0.25 = a quarter in.
    inset: 0.25,
    // The translucent fill behind the body and close-button pills.
    bodyBackground: "rgba(28, 33, 40, 0.85)",
    fontSize: 11,
  }}
/>
```

`extendLeft` (default `true`) controls whether the line continues past the label
to the left edge of the pane. Set it to `false` for a short line that only spans
from the label to the axis.

## Platform differences

The only difference is hover: on the web, moving the pointer over a line or its
close button brightens that segment (tune it with `priceLinesStyle.hoverBoost`,
or set it to `1` to disable). Touch has no hover state, so on React Native
everything renders at full strength all the time.

Escape-to-cancel is likewise web-only. On touch, drag the line back or reject the
move in `onPriceLineDragEnd`.

## Overlapping lines

Two lines at the same price stack on top of each other; vroom doesn't offset them
automatically. When several are within grab range of a press, the nearest one in
y wins, and a close button always wins over a line underneath it — so a mis-grab
can't cancel an order you weren't aiming at.
