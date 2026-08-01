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

Defined in: [types/src/index.ts:278](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L278)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:286](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L286)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types/src/index.ts:280](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L280)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types/src/index.ts:282](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L282)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types/src/index.ts:284](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L284)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:288](https://github.com/keepitreal/vroom/blob/3f6a67d6951af43fd8eed66e70bf2793c353bbc0/packages/types/src/index.ts#L288)

Stroke width in px. Default 1.5.
