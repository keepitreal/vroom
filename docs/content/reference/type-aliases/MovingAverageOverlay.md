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

Source: [types/src/index.ts:578](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L578)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:586](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L586)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:580](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L580)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:582](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L582)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:584](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L584)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:588](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L588)

Stroke width in px. Default 1.5.
