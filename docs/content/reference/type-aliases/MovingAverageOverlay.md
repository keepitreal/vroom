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

Source: [types/src/index.ts:290](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L290)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:298](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L298)

Line color (hex string or packed ARGB number).

---

### kind

```ts
kind: "sma" | "ema";
```

Source: [types/src/index.ts:292](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L292)

'sma' (simple) or 'ema' (exponential).

---

### length

```ts
length: number;
```

Source: [types/src/index.ts:294](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L294)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:296](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L296)

Price source. Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:300](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L300)

Stroke width in px. Default 1.5.
