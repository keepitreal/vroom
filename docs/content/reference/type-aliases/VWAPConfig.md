# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:362](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L362)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:368](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L368)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:364](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L364)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:366](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L366)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:370](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L370)

Stroke width in px. Default 1.5.
