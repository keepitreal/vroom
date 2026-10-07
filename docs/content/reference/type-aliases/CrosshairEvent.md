# `CrosshairEvent`

```ts
type CrosshairEvent = {
  active: boolean;
  candle: Candle | null;
  indicator: {
    kind: CrosshairIndicatorKind;
    value: number;
  } | null;
  price: number | null;
  reason: "show" | "move" | "hide";
  timeMs: number | null;
};
```

Source: [types/src/index.ts:27](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L27)

Payload passed to `onCrosshair` as the crosshair shows, moves, or hides.

## Properties

### active

```ts
active: boolean;
```

Source: [types/src/index.ts:29](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L29)

True while the crosshair is showing; false when it's dismissed.

---

### candle

```ts
candle: Candle | null;
```

Source: [types/src/index.ts:36](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L36)

OHLCV of the candle under the crosshair, or null when inactive. Also null
when the crosshair is parked on a _future_ candle-aligned slot in the empty
space ahead of the most recent candle (no candle exists there yet) — use
`timeMs` to read the slot's time in that case.

---

### indicator

```ts
indicator:
  | {
  kind: CrosshairIndicatorKind;
  value: number;
}
  | null;
```

Source: [types/src/index.ts:57](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L57)

The indicator pane under the crosshair and its value at the horizontal line
(what that pane's badge shows), or null over the price pane. `value` is in
the pane's own units: RSI on its 0–100 scale, MACD and ATR in price units.

---

### price

```ts
price: number | null;
```

Source: [types/src/index.ts:51](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L51)

Free price at the crosshair's horizontal line (what the price badge shows),
in data space. Null when inactive. Pair with `timeMs` to mirror this
crosshair onto another chart via its `crosshairOverride` prop.

Over a below-chart indicator pane the line isn't at a price level, so this
is the close of the candle under the vertical line instead — null on an
empty future slot.

---

### reason

```ts
reason: "show" | "move" | "hide";
```

Source: [types/src/index.ts:68](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L68)

Why this event fired — lets the host react differently (e.g. haptics):
'show' — long-press activated the crosshair
'move' — the crosshair moved: a different candle (time) or a vertical move
within the same candle (price). Fires on any positional change so
`price` stays current for cross-chart sync; for a per-candle
signal (e.g. haptics), dedupe on `timeMs` yourself.
'hide' — the crosshair was dismissed
The library never plays haptics itself; the host decides.

---

### timeMs

```ts
timeMs: number | null;
```

Source: [types/src/index.ts:41](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L41)

Bar-open time (Unix epoch ms) of the slot the crosshair snaps to, including
future candle-aligned slots past the last candle. Null when inactive.
