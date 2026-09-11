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

Source: [types/src/index.ts:510](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L510)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:518](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L518)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:512](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L512)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:514](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L514)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:516](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L516)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:520](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L520)

Stroke width in px. Default 1.5.
