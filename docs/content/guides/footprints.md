# Footprints

Footprints mark where a trade actually filled — the tracks a trader leaves across
their own chart. Each one draws as a small circular badge above the candle it
landed in: a `+` in the bull color for an entry, a `−` in the bear color for an
exit.

They work on **both web and React Native** (the renderer lives in the C++ core).

## A minimal set

```tsx
import { VroomChart, type Footprint } from "@vroomchart/react";

const fills: Footprint[] = [
  { id: "f1", timeMs: 1724198412000, side: "buy", price: 67120.5 },
  { id: "f2", timeMs: 1724201003000, side: "sell", price: 67980.0 },
];

<VroomChart candles={candles} footprints={fills} />;
```

`timeMs` is the **raw execution time**, not a bar-open time. Pass your fills
exactly as your venue reported them and let the chart place them.

## How fills land on candles

The chart buckets every footprint into whichever candle's window contains it —
the half-open range from that bar's open time to the next one's. Three
consequences worth knowing:

**One array serves every timeframe.** Switch from 1m to 1h and the badges
re-group onto the wider bars by themselves. You don't re-bucket anything, and you
don't hand the chart a different array per interval.

**A timestamp that doesn't match a bar still works.** Because placement is by
containment rather than equality, a fill at 09:31:47 shows up on the 09:30 bar of
a 5m series. Nothing is silently dropped for failing to line up.

**At most two badges render per candle.** One stands for that bar's buys and one
for its sells, however many trades went into each. A bar holding twenty fills
shows one or two badges and still reports all twenty when you hover it.

When a candle has both, the two badges stack upward above the high with a gap
between them, the side whose *latest* trade came first sitting nearer the bar.
The gap is there so each stays independently hoverable.

## Rendering your own tooltip

**The chart draws no tooltip.** It reports which badge is active, every trade on
that bar, and the geometry you need to place your own UI — then gets out of the
way. Your tooltip is your components, your styling, your data.

`onFootprint` fires when a badge is hovered (tapped on touch) or dismissed:

```ts
type FootprintEvent = {
  active: boolean;
  reason: "show" | "move" | "hide";
  side: FootprintSide | null;
  timeMs: number | null; // bar-open time of the candle
  footprints: Footprint[]; // every fill on that bar, both sides, by time
  badge: { x: number; y: number; radius: number } | null;
  pane: PlotRect | null; // the plot area, axis strips excluded
};
```

`badge` and `pane` are both in logical px relative to the chart element's
top-left, so they drop straight into an absolutely-positioned child of the same
container. `reason: 'move'` means the pointer slid from one badge to another
without leaving in between — worth distinguishing if your tooltip animates in.

### Why `pane` and not the element's own size

`pane` is the candle area: the element minus the price-axis and time-axis strips.
That distinction is the whole reason it's reported. Measure your container
instead and you'll believe there's room to the right of a badge near the edge,
because the last ~60px of the element is the price axis. The tooltip stops short
of the container and still lands on the axis labels.

### Choosing a side

Only you know how big your tooltip is, so the chart hands you two rects and lets
you decide. Testing a candidate placement is arithmetic:

```ts
const fitsRight = badge.x + badge.radius + 8 + width <= pane.right;
```

If you want a preference (`'top'`, `'left'`, `'right'`) that falls back when it
won't fit, that's a small pure function — no chart involvement:

```ts
import type { PlotRect } from "@vroomchart/react";

type Placement = "top" | "right" | "left";
type Badge = { x: number; y: number; radius: number };
type Size = { width: number; height: number };

// Where to try next when the preferred side doesn't fit.
const FALLBACKS: Record<Placement, Placement[]> = {
  right: ["right", "left", "top"],
  left: ["left", "right", "top"],
  top: ["top", "right", "left"],
};

function offsetFor(p: Placement, b: Badge, s: Size, gap: number) {
  switch (p) {
    case "right":
      return { left: b.x + b.radius + gap, top: b.y - s.height / 2 };
    case "left":
      return { left: b.x - b.radius - gap - s.width, top: b.y - s.height / 2 };
    case "top":
      return { left: b.x - s.width / 2, top: b.y - b.radius - gap - s.height };
  }
}

const clamp = (v: number, lo: number, hi: number) =>
  Math.min(Math.max(v, lo), Math.max(lo, hi));

export function placeTooltip(
  badge: Badge,
  pane: PlotRect,
  size: Size,
  prefer: Placement = "right",
  gap = 8,
) {
  for (const p of FALLBACKS[prefer]) {
    const at = offsetFor(p, badge, size, gap);
    if (
      at.left >= pane.left &&
      at.left + size.width <= pane.right &&
      at.top >= pane.top &&
      at.top + size.height <= pane.bottom
    ) {
      return { ...at, placement: p };
    }
  }
  // Nothing fits outright — honor the preference and pull it back inside.
  const at = offsetFor(prefer, badge, size, gap);
  return {
    left: clamp(at.left, pane.left, pane.right - size.width),
    top: clamp(at.top, pane.top, pane.bottom - size.height),
    placement: prefer,
  };
}
```

Returning the placement it settled on, not just coordinates, is what lets you
point a caret the right way when it flips.

### Putting it together

```tsx
const TOOLTIP = { width: 200, height: 64 };

function Chart({ candles, fills }) {
  const [hover, setHover] = useState<
    (FootprintEvent & { badge: Badge; pane: PlotRect }) | null
  >(null);

  const onFootprint = useCallback((e: FootprintEvent) => {
    // One guard covers dismissal and narrows the nullable geometry.
    setHover(e.active && e.badge && e.pane ? { ...e, badge: e.badge, pane: e.pane } : null);
  }, []);

  // The chart and the tooltip share a positioned container, which is the
  // coordinate space badge/pane are already in.
  return (
    <div style={{ position: "relative", flex: 1 }}>
      <VroomChart candles={candles} footprints={fills} onFootprint={onFootprint} />
      {hover && (
        <Tooltip
          {...placeTooltip(hover.badge, hover.pane, TOOLTIP, "right")}
          trades={hover.footprints.filter((f) => f.side === hover.side)}
        />
      )}
    </div>
  );
}
```

Give the tooltip `pointer-events: none` (`pointerEvents="none"` on React Native).
Without it, sliding onto the tooltip takes the pointer off the canvas, the chart
reports a dismissal, the tooltip unmounts, and the pointer is back over the badge
— a flicker loop. With it, the pointer passes through, which also means a tooltip
lying over the other badge of a stacked pair doesn't block it.

### Rejoining fills to your own data

`e.footprints` holds the same objects you passed in, so anything you hung off
them comes back:

```tsx
type MyFill = Footprint & { size: number; venue: string; orderId: string };

const mine = (e.footprints as MyFill[]).filter((f) => f.side === e.side);
const total = mine.reduce((sum, f) => sum + f.size, 0);
```

`price` is part of `Footprint` but the renderer ignores it — badges sit above the
bar, not at the fill — so it's carried through purely for your UI.

## Styling

Layout is shared across every badge via `footprintsStyle`; colors come from the
theme, so badges match the candles without being configured twice.

```tsx
<VroomChart
  candles={candles}
  footprints={fills}
  footprintsStyle={{
    radius: 9, // badge radius in px
    gap: 4, // between the two stacked badges on one bar
    margin: 8, // between the candle's high and the first badge
    hoverBoost: 1.25, // how much the hovered badge brightens
  }}
/>
```

The fills use `bull` and `bear` — the same colors as the candle bodies — and the
`+`/`−` glyph uses `badgeText`. Restyle them through `theme`.

## Platform differences

On the web, hovering a badge opens the tooltip and leaving the chart closes it.
Touch has no hover, so on React Native a **tap** opens one and the next tap
anywhere closes it; both arrive through the same `onFootprint` callback with the
same event shape.

`hoverBoost` and the halo ring are therefore web-only in practice. On React
Native the tapped badge still highlights, which is what marks the selection.
