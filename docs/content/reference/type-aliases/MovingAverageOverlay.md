# Type Alias: MovingAverageOverlay

```ts
type MovingAverageOverlay = {
  color?: string | number;
  kind: "sma" | "ema";
  length: number;
  source?: MASource;
  width?: number;
};
```

Defined in: [types/src/index.ts:102](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L102)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:110](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L110)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types/src/index.ts:104](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L104)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types/src/index.ts:106](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L106)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types/src/index.ts:108](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L108)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:112](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L112)

Stroke width in px. Default 1.5.
