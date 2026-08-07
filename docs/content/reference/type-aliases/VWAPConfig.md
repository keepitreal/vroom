# `VWAPConfig`

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:301](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L301)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:306](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L306)

Line color (hex string or packed ARGB number).

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:302](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L302)

---

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:304](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L304)

Session reset offset from UTC midnight, in minutes (default 0).

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:308](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L308)

Stroke width in px. Default 1.5.
