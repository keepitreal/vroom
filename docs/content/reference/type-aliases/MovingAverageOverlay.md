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

Defined in: [types/src/index.ts:176](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L176)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:184](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L184)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types/src/index.ts:178](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L178)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types/src/index.ts:180](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L180)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types/src/index.ts:182](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L182)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:186](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L186)

Stroke width in px. Default 1.5.
