# `MovingAverageOverlay`

```ts
type MovingAverageOverlay = {
  color?: string | number;
  kind: "sma" | "ema";
  length: number;
  source?: MASource;
  width?: number;
};
```

Source: [types/src/index.ts:278](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L278)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:286](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L286)

Line color (hex string or packed ARGB number).

---

### kind

```ts
kind: "sma" | "ema";
```

Source: [types/src/index.ts:280](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L280)

'sma' (simple) or 'ema' (exponential).

---

### length

```ts
length: number;
```

Source: [types/src/index.ts:282](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L282)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L284)

Price source. Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L288)

Stroke width in px. Default 1.5.
