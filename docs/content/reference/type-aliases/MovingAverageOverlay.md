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

Source: [types/src/index.ts:550](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L550)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:558](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L558)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:552](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L552)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:554](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L554)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:556](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L556)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:560](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L560)

Stroke width in px. Default 1.5.
