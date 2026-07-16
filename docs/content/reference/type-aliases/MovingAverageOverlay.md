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

Defined in: [types/src/index.ts:208](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L208)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:216](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L216)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types/src/index.ts:210](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L210)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types/src/index.ts:212](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L212)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types/src/index.ts:214](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L214)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:218](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L218)

Stroke width in px. Default 1.5.
