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

Source: [types/src/index.ts:542](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L542)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:550](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L550)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:544](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L544)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:546](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L546)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:548](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L548)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:552](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L552)

Stroke width in px. Default 1.5.
