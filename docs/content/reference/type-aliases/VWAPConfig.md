# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:595](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L595)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:601](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L601)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:597](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L597)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:599](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L599)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:603](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L603)

Stroke width in px. Default 1.5.
