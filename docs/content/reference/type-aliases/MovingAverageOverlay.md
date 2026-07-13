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

Defined in: [types/src/index.ts:168](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L168)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:176](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L176)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types/src/index.ts:170](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L170)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types/src/index.ts:172](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L172)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types/src/index.ts:174](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L174)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:178](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L178)

Stroke width in px. Default 1.5.
