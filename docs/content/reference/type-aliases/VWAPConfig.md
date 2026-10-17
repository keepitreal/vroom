# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:633](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L633)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:639](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L639)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:635](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L635)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:637](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L637)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:641](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L641)

Stroke width in px. Default 1.5.
