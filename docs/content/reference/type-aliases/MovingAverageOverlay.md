# `MovingAverageOverlay`

```ts
type MovingAverageOverlay = {
  color?: string | number;
  id?: string;
  maType: MAKind;
  period: number;
  source?: MASource;
  width?: number;
};
```

Source: [types/src/index.ts:681](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L681)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:694](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L694)

Line color (hex string or packed ARGB number).

---

### id?

```ts
optional id?: string;
```

Source: [types/src/index.ts:686](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L686)

Optional stable id, so a host or AI agent can update or remove one line in
the ribbon without matching on its settings. Not used by the renderer.

---

### maType

```ts
maType: MAKind;
```

Source: [types/src/index.ts:688](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L688)

Averaging for this line ([MAKind](MAKind.md)).

---

### period

```ts
period: number;
```

Source: [types/src/index.ts:690](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L690)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:692](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L692)

Price source ([MASource](MASource.md)). Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:696](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L696)

Stroke width in px. Default 1.5.
