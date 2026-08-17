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

Source: [types/src/index.ts:357](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L357)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:365](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L365)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:359](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L359)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:361](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L361)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:363](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L363)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:367](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L367)

Stroke width in px. Default 1.5.
