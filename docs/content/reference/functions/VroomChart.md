# Function: VroomChart()

```ts
function VroomChart(props): Element;
```

Defined in: [react-native/src/VroomChart.tsx:36](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/react-native/src/VroomChart.tsx#L36)

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
