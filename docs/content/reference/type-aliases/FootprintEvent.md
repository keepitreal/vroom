# `FootprintEvent`

```ts
type FootprintEvent = {
  active: boolean;
  badge: {
    radius: number;
    x: number;
    y: number;
  } | null;
  footprints: Footprint[];
  pane: PlotRect | null;
  reason: "show" | "move" | "hide";
  side: FootprintSide | null;
  timeMs: number | null;
};
```

Source: [types/src/index.ts:961](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L961)

Fired when the pointer enters, moves between, or leaves footprint badges (on
touch platforms, when one is tapped or dismissed).

The chart draws no tooltip of its own — this event is the hook for yours.
Position your UI off `badge` and `pane`, both in the same coordinate space as
the chart element, and fill it from `footprints`.

Panning or zooming fires a `'hide'`, since the bar the badge belongs to has
moved: you don't need your own gesture listener to take the tooltip down.

## Properties

### active

```ts
active: boolean;
```

Source: [types/src/index.ts:963](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L963)

True while a badge is hovered/tapped; false when it's dismissed.

---

### badge

```ts
badge:
  | {
  radius: number;
  x: number;
  y: number;
}
  | null;
```

Source: [types/src/index.ts:987](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L987)

The badge's center and radius in logical px relative to the chart element's
top-left — anchor your tooltip to it. Null when inactive.

---

### footprints

```ts
footprints: Footprint[];
```

Source: [types/src/index.ts:982](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L982)

Every footprint bucketed into that candle, _both_ sides, ascending by
`timeMs`. Empty when inactive. Filter on `side` to show only the hovered
badge's trades, or render the whole bar's activity at once.

---

### pane

```ts
pane: PlotRect | null;
```

Source: [types/src/index.ts:999](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L999)

The plot area the badge sits in, for choosing which side of it your tooltip
fits on. Null when inactive (there is nothing to place).

Only you know how big your tooltip is, so the chart reports the rect rather
than picking a side:

```ts
const fitsRight = badge.x + badge.radius + 8 + width <= pane.right;
```

---

### reason

```ts
reason: "show" | "move" | "hide";
```

Source: [types/src/index.ts:972](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L972)

Why this event fired:
'show' — a badge became hovered/tapped from nothing
'move' — the pointer moved to a _different_ badge without leaving in between
'hide' — the badge was dismissed: the pointer left (or a tap missed), the
chart was panned or zoomed out from under it, or the crosshair
took the pane over

---

### side

```ts
side: FootprintSide | null;
```

Source: [types/src/index.ts:974](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L974)

Which badge — its buys or its sells. Null when inactive.

---

### timeMs

```ts
timeMs: number | null;
```

Source: [types/src/index.ts:976](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L976)

Bar-open time (epoch ms) of the candle the badge sits on. Null when inactive.
