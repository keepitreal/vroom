# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:567](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L567)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:573](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L573)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:569](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L569)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:571](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L571)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:575](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L575)

Stroke width in px. Default 1.5.
