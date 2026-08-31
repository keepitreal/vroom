# `VroomTheme`

```ts
type VroomTheme = {
  accentBear?: VroomColor;
  accentBull?: VroomColor;
  axisText?: VroomColor;
  background?: VroomColor;
  badgeText?: VroomColor;
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
  lineTension?: number;
  lineTipDot?: boolean;
  lineTipPulse?: boolean;
  lineWidth?: number;
  showXAxis?: boolean;
  showYAxis?: boolean;
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

Source: [types/src/index.ts:101](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L101)

Axis label text (price + time).

---

### background?

```ts
optional background?: VroomColor;
```

Source: [types/src/index.ts:68](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L68)

Chart + axis-strip background.

---

### badgeText?

```ts
optional badgeText?: VroomColor;
```

Source: [types/src/index.ts:108](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L108)

Text drawn on a filled badge — the current-price indicator, the crosshair's
price and time badges, and price-line pills. Defaults to white, which reads
against the saturated fills those badges use on a dark theme; light themes
generally want a dark value here.

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

Source: [types/src/index.ts:110](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L110)

Crosshair dashed lines.

---

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Source: [types/src/index.ts:112](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L112)

Crosshair target — the hollow ring/dot at the intersection.

---

### grid?

```ts
optional grid?: VroomColor;
```

Source: [types/src/index.ts:99](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L99)

Gridlines.

---

### lineColor?

```ts
optional lineColor?: VroomColor;
```

Source: [types/src/index.ts:114](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L114)

Line-chart-mode close polyline color. Defaults to violet, matching the RSI line.

---

### lineGradientOpacity?

```ts
optional lineGradientOpacity?: number;
```

Source: [types/src/index.ts:122](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L122)

Opacity of the gradient filled beneath the line-chart polyline, at its
strongest point. The fill uses `lineColor` and ramps to fully transparent at
the bottom of the price pane. Defaults to 0.28; set to 0 to disable the fill.

---

### lineTension?

```ts
optional lineTension?: number;
```

Source: [types/src/index.ts:132](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L132)

How much to round the line chart's corners, from 0 (straight segments
between closes) to 1 (fully smooth). Defaults to 0.

The curve is monotone-limited, so smoothing can never overshoot into a price
that didn't trade: every peak and trough stays on an actual close, and the
curve never leaves the range of the two closes it connects. Applies to the
gradient fill beneath the line as well, so the two stay flush.

---

### lineTipDot?

```ts
optional lineTipDot?: boolean;
```

Source: [types/src/index.ts:141](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L141)

Mark the line chart's newest end with a dot. Defaults to `true`.

Takes its color from `line` and its radius from `lineWidth`, wrapped in a 2px
ring of `background` that separates it from the line itself. Only drawn in
line mode, and it crossfades along with the line during a candle↔line
transition.

---

### lineTipPulse?

```ts
optional lineTipPulse?: boolean;
```

Source: [types/src/index.ts:149](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L149)

Pulse a ring outward from the tip dot, once every 2.6s. Defaults to `false`.

Ignored when `lineTipDot` is off, and suppressed when the OS asks for reduced
motion. Because the ring never stops, enabling it keeps the chart repainting
continuously — leave it off for charts that should be able to go idle.

---

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:116](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L116)

Line-chart-mode polyline stroke width in px. Defaults to 1.5.

---

### showXAxis?

```ts
optional showXAxis?: boolean;
```

Source: [types/src/index.ts:169](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L169)

Show the time (x) axis strip along the bottom. Defaults to `true`.

Behaves like [VroomTheme.showYAxis](#showyaxis): the strip collapses, the plot
grows into it, and the time labels and crosshair time badge fade first.

---

### showYAxis?

```ts
optional showYAxis?: boolean;
```

Source: [types/src/index.ts:162](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L162)

Show the price (y) axis strip down the right edge. Defaults to `true`.

Hiding it collapses the strip and hands the reclaimed width to the plot, so
the candles widen to fill it. Its contents — price labels, the current-price
badge, the crosshair price badge, price-line axis badges — fade out ahead of
the collapse; gridlines and the price indicator's dotted line stay. Animated
over `transitionMs` unless the OS asks for reduced motion.

The axis drag-to-scale region goes with the strip, so a hidden axis can't be
scaled by dragging it.

---

### ~~volumeRadius?~~

```ts
optional volumeRadius?: number;
```

Source: [types/src/index.ts:97](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L97)

Corner radius (px) of the _top_ of volume bars. Defaults to 0 (square).

#### Deprecated

Use `volume.radius`, which sits with the rest of the volume
styling. This still applies when `volume.radius` is omitted.

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
