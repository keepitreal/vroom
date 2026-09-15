# `Footprint`

```ts
type Footprint = {
  id: string;
  price?: number;
  side: FootprintSide;
  timeMs: number;
};
```

Source: [types/src/index.ts:959](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L959)

A single filled trade, drawn as a circular badge above the candle it fell in —
the "footprint" a trader leaves on the chart: `buy` marks an entry (a `+`
badge in the bull color), `sell` marks an exit (a `−` badge in the bear color).

`timeMs` is the raw execution time, _not_ a bar-open time. The chart buckets
each footprint into whichever candle's window contains it, so the same array
renders correctly at every interval — switch from 1m to 1h and the badges
re-group onto the wider bars on their own.

At most two badges render per candle: one for that bar's buys and one for its
sells, however many trades went into each. Hovering (or tapping) a badge hands
every footprint on that candle back through `onFootprint`, so a bar holding
twenty fills still shows one badge and still reports all twenty.

## Properties

### id

```ts
id: string;
```

Source: [types/src/index.ts:961](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L961)

Stable unique id, echoed back in `onFootprint`.

---

### price?

```ts
optional price?: number;
```

Source: [types/src/index.ts:970](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L970)

Execution price. Ignored by the renderer (badges sit above the bar, not at
the fill), and carried through to `onFootprint` for your own UI.

---

### side

```ts
side: FootprintSide;
```

Source: [types/src/index.ts:965](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L965)

Entry (`'buy'`) or exit (`'sell'`) — picks the badge color and glyph.

---

### timeMs

```ts
timeMs: number;
```

Source: [types/src/index.ts:963](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L963)

Execution time as Unix epoch milliseconds, unsnapped.
