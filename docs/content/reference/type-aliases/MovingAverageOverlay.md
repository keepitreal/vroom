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

Source: [types/src/index.ts:559](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L559)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:567](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L567)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:561](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L561)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:563](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L563)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:565](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L565)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:569](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L569)

Stroke width in px. Default 1.5.
