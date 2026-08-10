# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:307](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L307)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:312](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L312)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:308](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L308)

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:310](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L310)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:314](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L314)

Stroke width in px. Default 1.5.
