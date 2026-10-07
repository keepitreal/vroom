# `CrosshairButtonConfig`

```ts
type CrosshairButtonConfig = {
  cornerRadius?: number;
  enabled?: boolean;
  hoverBoost?: number;
};
```

Source: [types/src/index.ts:1116](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1116)

The crosshair plus button: a "+" drawn inside the crosshair's price badge,
turning the whole badge into a button — the order-entry affordance.

Opt-in: nothing renders unless `enabled` is true. The badge takes its fill
and text color from the theme (`theme.crosshairTarget` / `theme.badgeText`)
and grows leftward to fit the plus, so the price stays in the y-axis column.
Clicking (or tapping) anywhere on it locks the crosshair at that price and
fires `onCrosshairButton`, which is where your own UI (e.g. "Buy limit" /
"Sell limit") comes in.

## Properties

### cornerRadius?

```ts
optional cornerRadius?: number;
```

Source: [types/src/index.ts:1124](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1124)

Badge corner radius in px, clamped to 0..height/2. Default 6 (the plain
badge's radius). Any value at or above half the height draws a fully
rounded pill.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:1118](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1118)

Show the plus inside the crosshair price badge. Default false.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1130](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1130)

How much the hovered (or open) badge brightens, as a channel multiplier.
1 disables the highlight. Default 1.25. Hover is web only — touch platforms
only see the open state.
