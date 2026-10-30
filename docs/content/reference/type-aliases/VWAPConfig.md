# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:703](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L703)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:709](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L709)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:705](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L705)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:707](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L707)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:711](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L711)

Stroke width in px. Default 1.5.
