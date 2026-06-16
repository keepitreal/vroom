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

Defined in: [types.ts:98](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L98)

A moving-average overlay line drawn on the price pane. Provide an array of
these via `movingAverages` to render a ribbon of SMA/EMA lines.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types.ts:106](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L106)

Line color (hex string or packed ARGB number).

***

### kind

```ts
kind: "sma" | "ema";
```

Defined in: [types.ts:100](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L100)

'sma' (simple) or 'ema' (exponential).

***

### length

```ts
length: number;
```

Defined in: [types.ts:102](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L102)

Lookback in candles.

***

### source?

```ts
optional source?: MASource;
```

Defined in: [types.ts:104](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L104)

Price source. Default 'close'.

***

### width?

```ts
optional width?: number;
```

Defined in: [types.ts:108](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L108)

Stroke width in px. Default 1.5.
