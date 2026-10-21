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

Source: [types/src/index.ts:625](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L625)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:633](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L633)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:627](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L627)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:629](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L629)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:631](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L631)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:635](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L635)

Stroke width in px. Default 1.5.
