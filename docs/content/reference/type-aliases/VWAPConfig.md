# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:295](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L295)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:300](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L300)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:296](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L296)

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:298](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L298)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:302](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L302)

Stroke width in px. Default 1.5.
