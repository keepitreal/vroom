---
title: "ScrollToLatestOptions"
description: "How ChartControls.scrollToLatest gets there."
---

```ts
type ScrollToLatestOptions = {
  animated?: boolean;
  durationMs?: number;
  easing?: TransitionEasing;
};
```

Source: [types/src/index.ts:323](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L323)

How [ChartControls.scrollToLatest](ChartControls.md#scrolltolatest) gets there.

## Properties

### animated?

```ts
optional animated?: boolean;
```

Source: [types/src/index.ts:328](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L328)

Glide to the newest candle rather than jumping. Default `true`. Always
jumps when the OS requests reduced motion or `reduceMotion` is set.

***

### durationMs?

```ts
optional durationMs?: number;
```

Source: [types/src/index.ts:330](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L330)

Length of the glide in ms. Default 500. `0` jumps.

***

### easing?

```ts
optional easing?: TransitionEasing;
```

Source: [types/src/index.ts:332](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L332)

Curve of the glide. Default `'ease-out'`.
