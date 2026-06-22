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

Defined in: [types/src/index.ts:48](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L48)

Color overrides for the chart, passed via the `theme` prop.

## Properties

### axisText?

```ts
optional axisText?: VroomColor;
```

Defined in: [types/src/index.ts:58](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L58)

Axis label text (price + time).

***

### background?

```ts
optional background?: VroomColor;
```

Defined in: [types/src/index.ts:50](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L50)

Chart + axis-strip background.

***

### bear?

```ts
optional bear?: VroomColor;
```

Defined in: [types/src/index.ts:54](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L54)

Down candles (also bear wicks, bear volume bars, falling price indicator).

***

### bull?

```ts
optional bull?: VroomColor;
```

Defined in: [types/src/index.ts:52](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L52)

Up candles (also bull wicks, bull volume bars, rising price indicator).

***

### crosshair?

```ts
optional crosshair?: VroomColor;
```

Defined in: [types/src/index.ts:60](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L60)

Crosshair dashed lines.

***

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Defined in: [types/src/index.ts:62](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L62)

Crosshair target — the hollow ring/dot at the intersection.

***

### grid?

```ts
optional grid?: VroomColor;
```

Defined in: [types/src/index.ts:56](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L56)

Gridlines.
