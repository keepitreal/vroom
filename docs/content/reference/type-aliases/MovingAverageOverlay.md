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

Source: [types/src/index.ts:529](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L529)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:537](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L537)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:531](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L531)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:533](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L533)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:535](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L535)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:539](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L539)

Stroke width in px. Default 1.5.
