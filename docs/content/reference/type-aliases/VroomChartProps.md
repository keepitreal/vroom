# `VroomChartProps`

```ts
type VroomChartProps = VroomChartCoreProps & {
  style?: StyleProp<ViewStyle>;
  transitionKey?: string | number;
};
```

Source: [react-native/src/types.ts:51](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L51)

Props for the [VroomChart](../functions/VroomChart.md) component. The cross-platform props come
from VroomChartCoreProps; `style` is the React Native flavor.

## Type Declaration

| Name             | Type                       | Description                                                                                                                                                                                                                                                                                                                                                           | Source                                                                                                               |
| ---------------- | -------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `style?`         | `StyleProp`\<`ViewStyle`\> | Style for the chart's root view. Defaults to filling the parent.                                                                                                                                                                                                                                                                                                      | [react-native/src/types.ts:53](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L53) |
| `transitionKey?` | `string` \| `number`       | Declares a timeframe switch: when it changes between renders on the same `seriesKey`, the data change morphs (`intervalTransition`) even when the candle step is unchanged or the data heuristics would call it a stream or a reset. With `visibleRange` set, this is the only way a switch morphs; the range is where it lands. Omit to rely on automatic detection. | [react-native/src/types.ts:61](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L61) |
