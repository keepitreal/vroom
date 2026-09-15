# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:559](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L559)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:565](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L565)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:561](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L561)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:563](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L563)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:567](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L567)

Stroke width in px. Default 1.5.
