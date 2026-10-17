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

Source: [types/src/index.ts:616](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L616)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:624](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L624)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:618](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L618)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:620](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L620)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:622](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L622)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:626](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L626)

Stroke width in px. Default 1.5.
