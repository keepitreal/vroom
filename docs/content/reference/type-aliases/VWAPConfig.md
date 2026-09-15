# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:558](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L558)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:564](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L564)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:560](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L560)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:562](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L562)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:566](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L566)

Stroke width in px. Default 1.5.
