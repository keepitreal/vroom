# `MovingAverageOverlay`

```ts
type MovingAverageOverlay = {
  color?: string | number;
  maType: MAKind;
  period: number;
  source?: MASource;
  width?: number;
};
```

Source: [types/src/index.ts:597](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L597)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:605](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L605)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:599](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L599)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:601](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L601)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:603](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L603)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:607](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L607)

Stroke width in px. Default 1.5.
