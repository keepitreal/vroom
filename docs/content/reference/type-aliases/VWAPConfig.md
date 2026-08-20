# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:401](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L401)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:407](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L407)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:403](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L403)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:405](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L405)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:409](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L409)

Stroke width in px. Default 1.5.
