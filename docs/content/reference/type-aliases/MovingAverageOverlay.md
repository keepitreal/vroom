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

Source: [types/src/index.ts:455](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L455)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:463](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L463)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:457](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L457)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:459](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L459)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:461](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L461)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:465](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L465)

Stroke width in px. Default 1.5.
