# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:582](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L582)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:588](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L588)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:584](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L584)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:586](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L586)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:590](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L590)

Stroke width in px. Default 1.5.
