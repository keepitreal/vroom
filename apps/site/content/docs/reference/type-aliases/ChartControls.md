---
title: "ChartControls"
description: "Programmatic controls for a mounted chart, received through the chartRef prop."
---

```ts
type ChartControls = {
  scrollToLatest(options?): void;
};
```

Source: [types/src/index.ts:339](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L339)

Programmatic controls for a mounted chart, received through the `chartRef`
prop.

## Methods

### scrollToLatest()

```ts
scrollToLatest(options?): void;
```

Source: [types/src/index.ts:349](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L349)

Scroll back so the newest candle sits at the right edge, keeping the
current zoom and leaving the price axis to its usual fit. A pan, pinch or
wheel during the glide takes over from it. Fires `onViewportChange` once
the view lands.

Pair with `onAwayFromLatestChange` to show your own "back to latest"
button only while the user is reading history.

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `options?` | [`ScrollToLatestOptions`](ScrollToLatestOptions.md) |

#### Returns

`void`
