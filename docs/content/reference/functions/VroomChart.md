# Function: VroomChart()

```ts
function VroomChart(props): Element;
```

Defined in: [react-native/src/VroomChart.tsx:36](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/react-native/src/VroomChart.tsx#L36)

Skia-rendered candlestick chart. Pass OHLCV `candles` and size it via `style`
(it fills its parent by default). Pan to scroll, pinch to zoom, drag the
price/time axes to rescale, and long-press for the crosshair. Optional
indicators (`rsi`, `macd`, `movingAverages`, `vwap`), colors (`theme`), and
events (`onCrosshair`, `onViewportChange`) are configured through props.

## Parameters

| Parameter | Type |
| ------ | ------ |
| `props` | [`VroomChartProps`](../type-aliases/VroomChartProps.md) |

## Returns

`Element`

## See

[VroomChartProps](../type-aliases/VroomChartProps.md) for the full prop reference.
