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

Source: [types/src/index.ts:295](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L295)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:303](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L303)

Line color (hex string or packed ARGB number).

---

### kind

```ts
kind: "sma" | "ema";
```

Source: [types/src/index.ts:297](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L297)

'sma' (simple) or 'ema' (exponential).

---

### length

```ts
length: number;
```

Source: [types/src/index.ts:299](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L299)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:301](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L301)

Price source. Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:305](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L305)

Stroke width in px. Default 1.5.
