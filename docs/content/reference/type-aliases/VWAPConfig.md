# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:632](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L632)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:638](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L638)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:634](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L634)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:636](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L636)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:640](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L640)

Stroke width in px. Default 1.5.
