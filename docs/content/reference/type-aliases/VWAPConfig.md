# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:312](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L312)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:317](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L317)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:313](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L313)

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:315](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L315)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:319](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L319)

Stroke width in px. Default 1.5.
