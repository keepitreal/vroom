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

Source: [types/src/index.ts:384](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L384)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:392](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L392)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:386](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L386)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:388](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L388)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:390](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L390)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:394](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L394)

Stroke width in px. Default 1.5.
