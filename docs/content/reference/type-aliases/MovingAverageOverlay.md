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

Source: [types/src/index.ts:502](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L502)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:510](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L510)

Line color (hex string or packed ARGB number).

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:504](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L504)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:506](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L506)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:508](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L508)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:512](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L512)

Stroke width in px. Default 1.5.
