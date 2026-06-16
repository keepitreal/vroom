# Type Alias: VroomChartProps

```ts
type VroomChartProps = {
  candles: Candle[];
  crosshairOffset?: number;
  height?: number;
  macd?: MACDConfig;
  movingAverages?: MovingAverageOverlay[];
  onCrosshair?: (e) => void;
  onViewportChange?: (startMs, endMs) => void;
  rsi?: RSIConfig;
  style?: StyleProp<ViewStyle>;
  theme?: VroomTheme;
  visibleRange?: VisibleRange;
  vwap?: VWAPConfig;
  width?: number;
};
```

Defined in: [types.ts:137](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L137)

Props for the [VroomChart](../functions/VroomChart.md) component.

## Properties

### candles

```ts
candles: Candle[];
```

Defined in: [types.ts:139](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L139)

OHLCV bars to render. The only required prop.

***

### crosshairOffset?

```ts
optional crosshairOffset?: number;
```

Defined in: [types.ts:165](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L165)

Pixels the crosshair dot / horizontal line sit *above* the touch point so
they aren't hidden under the thumb. The vertical line stays centered on the
touch x. Default 40.

***

### height?

```ts
optional height?: number;
```

Defined in: [types.ts:146](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L146)

***

### macd?

```ts
optional macd?: MACDConfig;
```

Defined in: [types.ts:155](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L155)

MACD indicator (its own pane below the candles). Omit/disable to hide it.

***

### movingAverages?

```ts
optional movingAverages?: MovingAverageOverlay[];
```

Defined in: [types.ts:157](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L157)

Moving-average overlay lines (SMA/EMA) drawn on the price pane.

***

### onCrosshair?

```ts
optional onCrosshair?: (e) => void;
```

Defined in: [types.ts:166](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L166)

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `e` | [`CrosshairEvent`](CrosshairEvent.md) |

#### Returns

`void`

***

### onViewportChange?

```ts
optional onViewportChange?: (startMs, endMs) => void;
```

Defined in: [types.ts:167](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L167)

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `startMs` | `number` |
| `endMs` | `number` |

#### Returns

`void`

***

### rsi?

```ts
optional rsi?: RSIConfig;
```

Defined in: [types.ts:153](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L153)

RSI indicator (pane below the candles). Omit/disable to hide it.

***

### style?

```ts
optional style?: StyleProp<ViewStyle>;
```

Defined in: [types.ts:148](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L148)

Style for the chart's root view. Defaults to filling the parent.

***

### theme?

```ts
optional theme?: VroomTheme;
```

Defined in: [types.ts:151](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L151)

***

### visibleRange?

```ts
optional visibleRange?: VisibleRange;
```

Defined in: [types.ts:150](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L150)

Time window to render. Omit (or both 0) to show every candle.

***

### vwap?

```ts
optional vwap?: VWAPConfig;
```

Defined in: [types.ts:159](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L159)

VWAP overlay (session anchor, configurable reset).

***

### width?

```ts
optional width?: number;
```

Defined in: [types.ts:145](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L145)

Explicit size overrides in logical px. When omitted, the chart fills its
parent (measured via onLayout). Use `style` (flex / aspectRatio / absolute
fill) for layout-driven sizing instead of hard-coding these.
