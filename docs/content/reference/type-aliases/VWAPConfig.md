# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:546](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L546)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:552](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L552)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:548](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L548)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:550](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L550)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:554](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L554)

Stroke width in px. Default 1.5.
