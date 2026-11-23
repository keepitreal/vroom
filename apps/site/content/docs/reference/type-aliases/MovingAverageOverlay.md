---
title: "MovingAverageOverlay"
description: "A moving-average overlay line drawn on the price pane."
---

```ts
type MovingAverageOverlay = {
  color?: string | number;
  id?: string;
  maType: MAKind;
  period: number;
  source?: MASource;
  width?: number;
};
```

Source: [types/src/index.ts:712](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L712)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:725](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L725)

Line color (hex string or packed ARGB number).

***

### id?

```ts
optional id?: string;
```

Source: [types/src/index.ts:717](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L717)

Optional stable id, so a host or AI agent can update or remove one line in
the ribbon without matching on its settings. Not used by the renderer.

***

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:719](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L719)

Averaging for this line ([MAKind](MAKind.md)).

***

### period

```ts
period: number;
```

Source: [types/src/index.ts:721](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L721)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:723](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L723)

Price source ([MASource](MASource.md)). Default 'close'.

***

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:727](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L727)

Stroke width in px. Default 1.5.
