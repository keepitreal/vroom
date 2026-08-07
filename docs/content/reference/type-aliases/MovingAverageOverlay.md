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

Source: [types/src/index.ts:284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L284)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Source: [types/src/index.ts:292](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L292)

Line color (hex string or packed ARGB number).

---

### kind

```ts
kind: "sma" | "ema";
```

Source: [types/src/index.ts:286](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L286)

'sma' (simple) or 'ema' (exponential).

---

### length

```ts
length: number;
```

Source: [types/src/index.ts:288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L288)

Lookback in candles.

---

### source?

```ts
optional source?: MASource;
```

Source: [types/src/index.ts:290](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L290)

Price source. Default 'close'.

---

### width?

```ts
optional width?: number;
```

Source: [types/src/index.ts:294](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L294)

Stroke width in px. Default 1.5.
