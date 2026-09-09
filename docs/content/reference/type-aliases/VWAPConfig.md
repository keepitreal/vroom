# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:519](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L519)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:525](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L525)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:521](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L521)

Draw the line. Default false.

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:523](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L523)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:527](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L527)

Stroke width in px. Default 1.5.
