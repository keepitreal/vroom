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

Source: [types/src/index.ts:475](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L475)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:483](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L483)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:477](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L477)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:479](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L479)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:481](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L481)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:485](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L485)

Stroke width in px. Default 1.5.
