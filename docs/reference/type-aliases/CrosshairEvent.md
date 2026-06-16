# Type Alias: CrosshairEvent

```ts
type CrosshairEvent = {
  active: boolean;
  candle: Candle | null;
  reason: "show" | "move" | "hide";
};
```

Defined in: [types.ts:20](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L20)

Payload passed to `onCrosshair` as the crosshair shows, moves, or hides.

## Properties

### active

```ts
active: boolean;
```

Defined in: [types.ts:22](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L22)

True while the crosshair is showing; false when it's dismissed.

***

### candle

```ts
candle: Candle | null;
```

Defined in: [types.ts:24](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L24)

OHLCV of the candle under the crosshair, or null when inactive.

***

### reason

```ts
reason: "show" | "move" | "hide";
```

Defined in: [types.ts:33](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L33)

Why this event fired — lets the host react differently (e.g. haptics):
  'show' — long-press activated the crosshair
  'move' — the crosshair snapped to a *different* candle (one per candle
           crossed; not per drag frame)
  'hide' — the crosshair was dismissed
The library never plays haptics itself; the host decides.
