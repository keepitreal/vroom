# Type Alias: CrosshairEvent

```ts
type CrosshairEvent = {
  active: boolean;
  candle: Candle | null;
  price: number | null;
  reason: "show" | "move" | "hide";
  timeMs: number | null;
};
```

Defined in: [types/src/index.ts:24](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L24)

Payload passed to `onCrosshair` as the crosshair shows, moves, or hides.

## Properties

### active

```ts
active: boolean;
```

Defined in: [types/src/index.ts:26](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L26)

True while the crosshair is showing; false when it's dismissed.

***

### candle

```ts
candle: Candle | null;
```

Defined in: [types/src/index.ts:33](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L33)

OHLCV of the candle under the crosshair, or null when inactive. Also null
when the crosshair is parked on a *future* candle-aligned slot in the empty
space ahead of the most recent candle (no candle exists there yet) — use
`timeMs` to read the slot's time in that case.

***

### price

```ts
price: number | null;
```

Defined in: [types/src/index.ts:44](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L44)

Free price at the crosshair's horizontal line (what the price badge shows),
in data space. Null when inactive. Pair with `timeMs` to mirror this
crosshair onto another chart via its `crosshairOverride` prop.

***

### reason

```ts
reason: "show" | "move" | "hide";
```

Defined in: [types/src/index.ts:55](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L55)

Why this event fired — lets the host react differently (e.g. haptics):
  'show' — long-press activated the crosshair
  'move' — the crosshair moved: a different candle (time) or a vertical move
           within the same candle (price). Fires on any positional change so
           `price` stays current for cross-chart sync; for a per-candle
           signal (e.g. haptics), dedupe on `timeMs` yourself.
  'hide' — the crosshair was dismissed
The library never plays haptics itself; the host decides.

***

### timeMs

```ts
timeMs: number | null;
```

Defined in: [types/src/index.ts:38](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L38)

Bar-open time (Unix epoch ms) of the slot the crosshair snaps to, including
future candle-aligned slots past the last candle. Null when inactive.
