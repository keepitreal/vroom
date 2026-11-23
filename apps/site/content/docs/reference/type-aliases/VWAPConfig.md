---
title: "VWAPConfig"
description: "VWAP overlay config (session anchor)."
---

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Source: [types/src/index.ts:733](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L733)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:739](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L739)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:735](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L735)

Draw the line. Default false.

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Source: [types/src/index.ts:737](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L737)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:741](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L741)

Stroke width in px. Default 1.5.
