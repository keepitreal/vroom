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

Source: [types/src/index.ts:519](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L519)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:527](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L527)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:521](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L521)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:523](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L523)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:525](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L525)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:529](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L529)

Stroke width in px. Default 1.5.
