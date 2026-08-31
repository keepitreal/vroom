# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:492](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L492)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:498](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L498)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:494](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L494)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:496](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L496)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:500](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L500)

Stroke width in px. Default 1.5.
