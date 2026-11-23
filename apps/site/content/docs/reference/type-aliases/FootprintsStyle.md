---
title: "FootprintsStyle"
description: "Shared layout/style for every footprint badge, passed via footprintsStyle."
---

```ts
type FootprintsStyle = {
  gap?: number;
  hoverBoost?: number;
  margin?: number;
  radius?: number;
};
```

Source: [types/src/index.ts:1171](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1171)

Shared layout/style for every footprint badge, passed via `footprintsStyle`.

## Properties

### ~~gap?~~

```ts
optional gap?: number;
```

Source: [types/src/index.ts:1178](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1178)

#### Deprecated

Unused: a candle with both a buy and a sell now draws one
combined badge, so there is no stack to space out.

***

### hoverBoost?

```ts
optional hoverBoost?: number;
```

Source: [types/src/index.ts:1188](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1188)

How much the hovered badge brightens, as a channel multiplier. 1 disables
the highlight (the halo ring still draws). Default 1.25.

***

### margin?

```ts
optional margin?: number;
```

Source: [types/src/index.ts:1183](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1183)

Gap between the candle's high and the badge, in px, in candle mode. Default
8. In line mode the badge sits on the line instead.

***

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:1173](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1173)

Badge radius in px. Default 9.
