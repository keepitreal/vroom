# `CrosshairConfig`

```ts
type CrosshairConfig = {
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

Source: [types/src/index.ts:1220](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1220)

Crosshair behavior and look. Every field is optional; colors come from the
theme (`crosshair` for the lines, `crosshairTarget` for the ring and badge
fill, `badgeText` for badge text).

## Properties

### button?

```ts
optional button?: CrosshairButtonConfig;
```

Source: [types/src/index.ts:1237](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1237)

The plus button (advanced mode only). Replaces `crosshairButton`.

---

### hour12?

```ts
optional hour12?: boolean;
```

Source: [types/src/index.ts:1230](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1230)

12-hour clock with AM/PM (`11:30 AM`). Default false.

---

### lineStyle?

```ts
optional lineStyle?: CrosshairLineStyle;
```

Source: [types/src/index.ts:1224](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1224)

Default `'dashed'` in advanced mode, `'solid'` in simple mode.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:1226](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1226)

Line stroke in px. Default 1. Dash and dot spacing scale with it.

---

### mode?

```ts
optional mode?: CrosshairMode;
```

Source: [types/src/index.ts:1222](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1222)

Default `'advanced'`. See [CrosshairMode](CrosshairMode.md).

---

### offset?

```ts
optional offset?: number;
```

Source: [types/src/index.ts:1235](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1235)

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

Source: [types/src/index.ts:1239](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1239)

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

Source: [types/src/index.ts:1228](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1228)

Default `'datetime'` in advanced mode, `'time'` in simple mode.
