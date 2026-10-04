# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:576](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L576)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:582](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L582)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:578](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L578)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:580](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L580)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:584](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L584)

Stroke width in px. Default 1.5.
