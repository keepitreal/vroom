# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:374](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L374)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:380](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L380)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:376](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L376)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:378](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L378)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:382](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L382)

Stroke width in px. Default 1.5.
