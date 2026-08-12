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

Source: [types/src/index.ts:345](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L345)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:353](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L353)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:347](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L347)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:349](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L349)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:351](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L351)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:355](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L355)

Stroke width in px. Default 1.5.
