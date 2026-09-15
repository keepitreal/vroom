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

Source: [types/src/index.ts:541](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L541)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:549](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L549)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:543](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L543)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:545](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L545)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:547](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L547)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:551](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L551)

Stroke width in px. Default 1.5.
