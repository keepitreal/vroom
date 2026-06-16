# Type Alias: VroomTheme

```ts
type VroomTheme = {
  axisText?: VroomColor;
  background?: VroomColor;
  bear?: VroomColor;
  bull?: VroomColor;
  crosshair?: VroomColor;
  crosshairTarget?: VroomColor;
  grid?: VroomColor;
};
```

Defined in: [types.ts:44](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L44)

Color overrides for the chart, passed via the `theme` prop.

## Properties

### axisText?

```ts
optional axisText?: VroomColor;
```

Defined in: [types.ts:54](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L54)

Axis label text (price + time).

***

### background?

```ts
optional background?: VroomColor;
```

Defined in: [types.ts:46](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L46)

Chart + axis-strip background.

***

### bear?

```ts
optional bear?: VroomColor;
```

Defined in: [types.ts:50](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L50)

Down candles (also bear wicks, bear volume bars, falling price indicator).

***

### bull?

```ts
optional bull?: VroomColor;
```

Defined in: [types.ts:48](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L48)

Up candles (also bull wicks, bull volume bars, rising price indicator).

***

### crosshair?

```ts
optional crosshair?: VroomColor;
```

Defined in: [types.ts:56](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L56)

Crosshair dashed lines.

***

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Defined in: [types.ts:58](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L58)

Crosshair target — the hollow ring/dot at the intersection.

***

### grid?

```ts
optional grid?: VroomColor;
```

Defined in: [types.ts:52](https://github.com/keepitreal/vroom/blob/5754f44f04cecef0138d960a9d6cb9811c7231fa/packages/react-native/src/types.ts#L52)

Gridlines.
