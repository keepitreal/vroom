# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:472](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L472)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:478](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L478)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:474](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L474)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:476](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L476)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:480](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L480)

Stroke width in px. Default 1.5.
