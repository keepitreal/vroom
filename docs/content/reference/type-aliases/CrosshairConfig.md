# `CrosshairConfig`

```ts
type CrosshairConfig = {
  badgeColor?: VroomColor;
  badgeTextColor?: VroomColor;
  button?: CrosshairButtonConfig;
  hour12?: boolean;
  lineStyle?: CrosshairLineStyle;
  lineWidth?: number;
  mode?: CrosshairMode;
  offset?: number;
  simple?: {
    dimAhead?: boolean;
    dimOpacity?: number;
    height?: number;
  };
  timeFormat?: CrosshairTimeFormat;
};
```

Source: [types/src/index.ts:1267](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1267)

Crosshair behavior and look. Every field is optional; colors come from the
theme (`crosshair` for the lines, `crosshairTarget` for the ring and badge
fill, `badgeText` for badge text) unless `badgeColor` / `badgeTextColor` set
the badge fill / text.

## Properties

### badgeColor?

```ts
optional badgeColor?: VroomColor;
```

Source: [types/src/index.ts:1283](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1283)

Fill of the crosshair's time and price badges, the plus button and the
simple-mode badge. Defaults to `theme.crosshairTarget`, which also colors
the ring; this leaves the ring alone. Text follows `badgeTextColor`.

---

### badgeTextColor?

```ts
optional badgeTextColor?: VroomColor;
```

Source: [types/src/index.ts:1288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1288)

Text on the crosshair's time and price badges, the plus button and the
simple-mode badge. Defaults to `theme.badgeText`.

---

### button?

```ts
optional button?: CrosshairButtonConfig;
```

Source: [types/src/index.ts:1295](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1295)

The plus button (advanced mode only). Replaces `crosshairButton`.

---

### hour12?

```ts
optional hour12?: boolean;
```

Source: [types/src/index.ts:1277](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1277)

12-hour clock with AM/PM (`11:30 AM`). Default false.

---

### lineStyle?

```ts
optional lineStyle?: CrosshairLineStyle;
```

Source: [types/src/index.ts:1271](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1271)

Default `'dashed'` in advanced mode, `'solid'` in simple mode.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1273](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1273)

Line stroke in px. Default 1. Dash and dot spacing scale with it.

---

### mode?

```ts
optional mode?: CrosshairMode;
```

Source: [types/src/index.ts:1269](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1269)

Default `'advanced'`. See [CrosshairMode](CrosshairMode.md).

---

### offset?

```ts
optional offset?: number;
```

Source: [types/src/index.ts:1293](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1293)

Pixels the touch crosshair's dot and horizontal line sit above the finger
(advanced mode). Default 40. Replaces `crosshairOffset`.

---

### simple?

```ts
optional simple?: {
  dimAhead?: boolean;
  dimOpacity?: number;
  height?: number;
};
```

Source: [types/src/index.ts:1297](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1297)

Simple-mode options.

#### dimAhead?

```ts
optional dimAhead?: boolean;
```

Dim everything right of the line, the scrub highlight. Default true.

#### dimOpacity?

```ts
optional dimOpacity?: number;
```

Opacity of what `dimAhead` dims, 0 to 1. Default 0.35.

#### height?

```ts
optional height?: number;
```

Where the badge's top sits, as a fraction of the chart's height: 1 is the
top, 0 the bottom. Default 1. The badge always stays fully on screen, and
the line runs from under it down to the bottom of the plot.

---

### timeFormat?

```ts
optional timeFormat?: CrosshairTimeFormat;
```

Source: [types/src/index.ts:1275](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1275)

Default `'datetime'` in advanced mode, `'time'` in simple mode.
