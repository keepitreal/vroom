# `CrosshairButtonConfig`

```ts
type CrosshairButtonConfig = {
  background?: VroomColor;
  cornerRadius?: number;
  enabled?: boolean;
  gap?: number;
  hoverBoost?: number;
  iconColor?: VroomColor;
  iconStrokeWidth?: number;
  ring?: boolean;
  ringColor?: VroomColor;
  size?: number;
};
```

Source: [types/src/index.ts:1094](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1094)

The crosshair plus button: a small button on the crosshair's horizontal line,
directly left of the price badge — TradingView's order-entry affordance.

Opt-in: nothing renders unless `enabled` is true. Clicking (or tapping) it
locks the crosshair at that price and fires `onCrosshairButton`, which is
where your own UI (e.g. "Buy limit" / "Sell limit") comes in. Every style
field is optional.

## Properties

### background?

```ts
optional background?: VroomColor;
```

Source: [types/src/index.ts:1105](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1105)

Container fill. Defaults to the price badge's fill (`theme.crosshairTarget`).

---

### cornerRadius?

```ts
optional cornerRadius?: number;
```

Source: [types/src/index.ts:1103](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1103)

Container corner radius in px, clamped to 0..size/2. Default 4. `size / 2`
draws a circle; 0 draws square corners.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:1096](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1096)

Show the button. Default false.

---

### gap?

```ts
optional gap?: number;
```

Source: [types/src/index.ts:1115](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1115)

Gap between the button and the price badge in px. Default 4.

---

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1120](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1120)

How much the hovered button brightens, as a channel multiplier. 1 disables
the highlight. Default 1.25. Web only — touch platforms have no hover state.

---

### iconColor?

```ts
optional iconColor?: VroomColor;
```

Source: [types/src/index.ts:1107](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1107)

Plus glyph color. Defaults to the badge text color (`theme.badgeText`).

---

### iconStrokeWidth?

```ts
optional iconStrokeWidth?: number;
```

Source: [types/src/index.ts:1109](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1109)

Plus glyph stroke width in px. Default 1.5.

---

### ring?

```ts
optional ring?: boolean;
```

Source: [types/src/index.ts:1111](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1111)

Draw a circle around the plus, TradingView style. Default true.

---

### ringColor?

```ts
optional ringColor?: VroomColor;
```

Source: [types/src/index.ts:1113](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1113)

Ring color. Defaults to `iconColor`.

---

### size?

```ts
optional size?: number;
```

Source: [types/src/index.ts:1098](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1098)

Side of the square container in px. Default 20.
