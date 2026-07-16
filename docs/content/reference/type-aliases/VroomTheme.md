# Type Alias: VroomTheme

```ts
type VroomTheme = {
  accentBear?: VroomColor;
  accentBull?: VroomColor;
  axisText?: VroomColor;
  background?: VroomColor;
  bear?: VroomColor;
  borderBear?: VroomColor;
  borderBull?: VroomColor;
  bull?: VroomColor;
  candleRadius?: number;
  crosshair?: VroomColor;
  crosshairTarget?: VroomColor;
  grid?: VroomColor;
  volumeRadius?: number;
  wickBear?: VroomColor;
  wickBull?: VroomColor;
  wickRoundCap?: boolean;
  wickWidth?: number;
};
```

Defined in: [types/src/index.ts:66](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L66)

Color overrides for the chart, passed via the `theme` prop.

## Properties

### accentBear?

```ts
optional accentBear?: VroomColor;
```

Defined in: [types/src/index.ts:76](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L76)

Generic down color for the price indicator, volume bars, and MACD histogram. Defaults to red; independent of `bear`.

***

### accentBull?

```ts
optional accentBull?: VroomColor;
```

Defined in: [types/src/index.ts:74](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L74)

Generic up color for the price indicator, volume bars, and MACD histogram. Defaults to teal-green; independent of `bull`.

***

### axisText?

```ts
optional axisText?: VroomColor;
```

Defined in: [types/src/index.ts:96](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L96)

Axis label text (price + time).

***

### background?

```ts
optional background?: VroomColor;
```

Defined in: [types/src/index.ts:68](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L68)

Chart + axis-strip background.

***

### bear?

```ts
optional bear?: VroomColor;
```

Defined in: [types/src/index.ts:72](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L72)

Down candle body fill. Wick and border default to this unless overridden.

***

### borderBear?

```ts
optional borderBear?: VroomColor;
```

Defined in: [types/src/index.ts:80](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L80)

Down candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bear fill color to hide it.

***

### borderBull?

```ts
optional borderBull?: VroomColor;
```

Defined in: [types/src/index.ts:78](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L78)

Up candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bull fill color to hide it.

***

### bull?

```ts
optional bull?: VroomColor;
```

Defined in: [types/src/index.ts:70](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L70)

Up candle body fill. Wick and border default to this unless overridden.

***

### candleRadius?

```ts
optional candleRadius?: number;
```

Defined in: [types/src/index.ts:88](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L88)

Corner radius (px) of candle bodies. Defaults to 0 (square).

***

### crosshair?

```ts
optional crosshair?: VroomColor;
```

Defined in: [types/src/index.ts:98](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L98)

Crosshair dashed lines.

***

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Defined in: [types/src/index.ts:100](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L100)

Crosshair target — the hollow ring/dot at the intersection.

***

### grid?

```ts
optional grid?: VroomColor;
```

Defined in: [types/src/index.ts:94](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L94)

Gridlines.

***

### volumeRadius?

```ts
optional volumeRadius?: number;
```

Defined in: [types/src/index.ts:92](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L92)

Corner radius (px) of the *top* of volume bars. Defaults to 0 (square).

***

### wickBear?

```ts
optional wickBear?: VroomColor;
```

Defined in: [types/src/index.ts:84](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L84)

Down candle wick color. Defaults to the bear fill color.

***

### wickBull?

```ts
optional wickBull?: VroomColor;
```

Defined in: [types/src/index.ts:82](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L82)

Up candle wick color. Defaults to the bull fill color.

***

### wickRoundCap?

```ts
optional wickRoundCap?: boolean;
```

Defined in: [types/src/index.ts:90](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L90)

Round the wick end caps. Defaults to false.

***

### wickWidth?

```ts
optional wickWidth?: number;
```

Defined in: [types/src/index.ts:86](https://github.com/keepitreal/vroom/blob/6943052c3fb56f2a2d5cde5a473c2c9823a34ef0/packages/types/src/index.ts#L86)

Wick stroke width in px (applies to both up and down wicks). Defaults to 1.
