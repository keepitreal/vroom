# Type Alias: CrosshairEvent

```ts
type CrosshairEvent = {
  active: boolean;
  candle: Candle | null;
  timeMs: number | null;
  reason: "show" | "move" | "hide";
};
```

Defined in: [types/src/index.ts:24](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L24)

Payload passed to `onCrosshair` as the crosshair shows, moves, or hides.

## Properties

### active

```ts
active: boolean;
```

Defined in: [types/src/index.ts:26](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L26)

True while the crosshair is showing; false when it's dismissed.

***

### candle

```ts
candle: Candle | null;
```

Defined in: [types/src/index.ts:28](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L28)

OHLCV of the candle under the crosshair, or null when inactive. Also null when
the crosshair is parked on a future candle-aligned slot in the empty space ahead
of the most recent candle (no candle exists there yet) — use `timeMs` to read
the slot's time in that case.

***

### timeMs

```ts
timeMs: number | null;
```

Defined in: [types/src/index.ts:35](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L35)

Bar-open time (Unix epoch ms) of the slot the crosshair snaps to, including
future candle-aligned slots past the last candle. Null when inactive.

***

### reason

```ts
reason: "show" | "move" | "hide";
```

Defined in: [types/src/index.ts:37](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L37)

Why this event fired — lets the host react differently (e.g. haptics):
  'show' — long-press activated the crosshair
  'move' — the crosshair snapped to a *different* candle (one per candle
           crossed; not per drag frame)
  'hide' — the crosshair was dismissed
The library never plays haptics itself; the host decides.
