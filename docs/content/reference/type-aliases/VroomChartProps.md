# Type Alias: VroomChartProps

```ts
type VroomChartProps = VroomChartCoreProps & {
  style?: StyleProp<ViewStyle>;
};
```

Defined in: [react-native/src/types.ts:26](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L26)

Props for the [VroomChart](../functions/VroomChart.md) component. The cross-platform props come
from VroomChartCoreProps; `style` is the React Native flavor.

## Type Declaration

| Name | Type | Description | Defined in |
| ------ | ------ | ------ | ------ |
| `style?` | `StyleProp`\<`ViewStyle`\> | Style for the chart's root view. Defaults to filling the parent. | [react-native/src/types.ts:28](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L28) |
