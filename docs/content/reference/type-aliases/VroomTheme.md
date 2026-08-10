# `VroomTheme`

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
  lineColor?: VroomColor;
  lineGradientOpacity?: number;
  lineWidth?: number;
  volumeRadius?: number;
  wickBear?: VroomColor;
  wickBull?: VroomColor;
  wickRoundCap?: boolean;
  wickWidth?: number;
};
```

Source: [types/src/index.ts:66](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L66)

Color overrides for the chart, passed via the `theme` prop.

## Properties

### accentBear?

```ts
optional accentBear?: VroomColor;
```

Source: [types/src/index.ts:76](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L76)

Generic down color for the price indicator, volume bars, and MACD histogram. Defaults to red; independent of `bear`.

---

### accentBull?

```ts
optional accentBull?: VroomColor;
```

Source: [types/src/index.ts:74](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L74)

Generic up color for the price indicator, volume bars, and MACD histogram. Defaults to teal-green; independent of `bull`.

---

### axisText?

```ts
optional axisText?: VroomColor;
```

Source: [types/src/index.ts:96](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L96)

Axis label text (price + time).

---

### background?

```ts
optional background?: VroomColor;
```

Source: [types/src/index.ts:68](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L68)

Chart + axis-strip background.

---

### bear?

```ts
optional bear?: VroomColor;
```

Source: [types/src/index.ts:72](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L72)

Down candle body fill. Wick and border default to this unless overridden.

---

### borderBear?

```ts
optional borderBear?: VroomColor;
```

Source: [types/src/index.ts:80](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L80)

Down candle body border (1px, drawn _inside_ the body so it never changes candle width). Omit or set to the bear fill color to hide it.

---

### borderBull?

```ts
optional borderBull?: VroomColor;
```

Source: [types/src/index.ts:78](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L78)

Up candle body border (1px, drawn _inside_ the body so it never changes candle width). Omit or set to the bull fill color to hide it.

---

### bull?

```ts
optional bull?: VroomColor;
```

Source: [types/src/index.ts:70](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L70)

Up candle body fill. Wick and border default to this unless overridden.

---

### candleRadius?

```ts
optional candleRadius?: number;
```

Source: [types/src/index.ts:88](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L88)

Corner radius (px) of candle bodies. Defaults to 0 (square).

---

### crosshair?

```ts
optional crosshair?: VroomColor;
```

Source: [types/src/index.ts:98](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L98)

Crosshair dashed lines.

---

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Source: [types/src/index.ts:100](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L100)

Crosshair target — the hollow ring/dot at the intersection.

---

### grid?

```ts
optional grid?: VroomColor;
```

Source: [types/src/index.ts:94](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L94)

Gridlines.

---

### lineColor?

```ts
optional lineColor?: VroomColor;
```

Source: [types/src/index.ts:102](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L102)

Line-chart-mode close polyline color. Defaults to violet, matching the RSI line.

---

### lineGradientOpacity?

```ts
optional lineGradientOpacity?: number;
```

Source: [types/src/index.ts:110](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L110)

Opacity of the gradient filled beneath the line-chart polyline, at its
strongest point. The fill uses `lineColor` and ramps to fully transparent at
the bottom of the price pane. Defaults to 0.28; set to 0 to disable the fill.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:104](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L104)

Line-chart-mode polyline stroke width in px. Defaults to 1.5.

---

### volumeRadius?

```ts
optional volumeRadius?: number;
```

Source: [types/src/index.ts:92](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L92)

Corner radius (px) of the _top_ of volume bars. Defaults to 0 (square).

---

### wickBear?

```ts
optional wickBear?: VroomColor;
```

Source: [types/src/index.ts:84](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L84)

Down candle wick color. Defaults to the bear fill color.

---

### wickBull?

```ts
optional wickBull?: VroomColor;
```

Source: [types/src/index.ts:82](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L82)

Up candle wick color. Defaults to the bull fill color.

---

### wickRoundCap?

```ts
optional wickRoundCap?: boolean;
```

Source: [types/src/index.ts:90](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L90)

Round the wick end caps. Defaults to false.

---

### wickWidth?

```ts
optional wickWidth?: number;
```

Source: [types/src/index.ts:86](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L86)

Wick stroke width in px (applies to both up and down wicks). Defaults to 1.
