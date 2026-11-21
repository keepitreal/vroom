---
title: "VroomTheme"
description: "Color overrides for the chart, passed via the theme prop."
---

```ts
type VroomTheme = {
  accentBear?: VroomColor;
  accentBull?: VroomColor;
  axisFontSize?: number;
  axisText?: VroomColor;
  background?: VroomColor;
  badgeFontData?: ArrayBuffer;
  badgeFontSize?: number;
  badgeText?: VroomColor;
  bear?: VroomColor;
  borderBear?: VroomColor;
  borderBull?: VroomColor;
  bull?: VroomColor;
  candleRadius?: number;
  crosshair?: VroomColor;
  crosshairTarget?: VroomColor;
  fontData?: ArrayBuffer;
  grid?: VroomColor;
  lineColor?: VroomColor;
  lineGradientOpacity?: number;
  lineTension?: number;
  lineTipDot?: boolean;
  lineTipDotRadius?: number;
  lineTipPulse?: boolean;
  lineTipPulseOpacity?: number;
  lineWidth?: number;
  minPriceRangeRatio?: number;
  priceDecimals?: number;
  priceIndicatorBadgeText?: VroomColor;
  priceIndicatorDashGap?: number;
  priceIndicatorDashLength?: number;
  priceIndicatorLineOpacity?: number;
  priceIndicatorLineWidth?: number;
  priceIndicatorRoundCap?: boolean;
  showXAxis?: boolean;
  showYAxis?: boolean;
  simpleCrosshairBadgeFontSize?: number;
  skeleton?: VroomColor;
  volumeRadius?: number;
  wickBear?: VroomColor;
  wickBull?: VroomColor;
  wickRoundCap?: boolean;
  wickWidth?: number;
  yAxisFade?: boolean;
};
```

Source: [types/src/index.ts:79](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L79)

Color overrides for the chart, passed via the `theme` prop.

## Properties

### accentBear?

```ts
optional accentBear?: VroomColor;
```

Source: [types/src/index.ts:89](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L89)

Generic down color for the price indicator, volume bars, and MACD histogram. Defaults to red; independent of `bear`.

***

### accentBull?

```ts
optional accentBull?: VroomColor;
```

Source: [types/src/index.ts:87](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L87)

Generic up color for the price indicator, volume bars, and MACD histogram. Defaults to teal-green; independent of `bull`.

***

### axisFontSize?

```ts
optional axisFontSize?: number;
```

Source: [types/src/index.ts:121](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L121)

Axis label size in integer CSS px, clamped to 10–14. Omit for 11.

Shared by the price ticks, the time labels and the labels in indicator
panes (RSI, MACD, ATR). Badges follow it too unless `badgeFontSize` is set.

***

### axisText?

```ts
optional axisText?: VroomColor;
```

Source: [types/src/index.ts:114](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L114)

Axis label text (price + time).

***

### background?

```ts
optional background?: VroomColor;
```

Source: [types/src/index.ts:81](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L81)

Chart + axis-strip background.

***

### badgeFontData?

```ts
optional badgeFontData?: ArrayBuffer;
```

Source: [types/src/index.ts:143](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L143)

React Native only. A .ttf/.otf file's bytes for the badges `badgeFontSize`
sizes. Omit to follow `fontData`.

***

### badgeFontSize?

```ts
optional badgeFontSize?: number;
```

Source: [types/src/index.ts:128](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L128)

Text size on filled badges in integer CSS px, clamped to 10–14: the
current-price badge, the crosshair's price and time badges, and price-line
pills (unless `priceLinesStyle.fontSize` sets their own). Omit or pass 0 to
follow `axisFontSize`.

***

### badgeText?

```ts
optional badgeText?: VroomColor;
```

Source: [types/src/index.ts:154](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L154)

Text drawn on a filled badge — the current-price indicator, the crosshair's
price and time badges, and price-line pills. Defaults to white, which reads
against the saturated fills those badges use on a dark theme; light themes
generally want a dark value here.

Each of those badges can override it: `priceIndicatorBadgeText` for the
current-price badge, `crosshair.badgeTextColor` for the crosshair's badges,
and `badgeTextColor` on an individual [PriceLine](PriceLine.md).

***

### bear?

```ts
optional bear?: VroomColor;
```

Source: [types/src/index.ts:85](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L85)

Down candle body fill. Wick and border default to this unless overridden.

***

### borderBear?

```ts
optional borderBear?: VroomColor;
```

Source: [types/src/index.ts:93](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L93)

Down candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bear fill color to hide it.

***

### borderBull?

```ts
optional borderBull?: VroomColor;
```

Source: [types/src/index.ts:91](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L91)

Up candle body border (1px, drawn *inside* the body so it never changes candle width). Omit or set to the bull fill color to hide it.

***

### bull?

```ts
optional bull?: VroomColor;
```

Source: [types/src/index.ts:83](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L83)

Up candle body fill. Wick and border default to this unless overridden.

***

### candleRadius?

```ts
optional candleRadius?: number;
```

Source: [types/src/index.ts:101](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L101)

Corner radius (px) of candle bodies. Defaults to 0 (square).

***

### crosshair?

```ts
optional crosshair?: VroomColor;
```

Source: [types/src/index.ts:156](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L156)

Crosshair dashed lines.

***

### crosshairTarget?

```ts
optional crosshairTarget?: VroomColor;
```

Source: [types/src/index.ts:158](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L158)

Crosshair target — the hollow ring/dot at the intersection.

***

### fontData?

```ts
optional fontData?: ArrayBuffer;
```

Source: [types/src/index.ts:138](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L138)

React Native only. A .ttf/.otf file's bytes for the chart's text, e.g. axis
labels. Omit for the system font.

***

### grid?

```ts
optional grid?: VroomColor;
```

Source: [types/src/index.ts:112](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L112)

Gridlines.

***

### lineColor?

```ts
optional lineColor?: VroomColor;
```

Source: [types/src/index.ts:160](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L160)

Line-chart-mode close polyline color. Defaults to violet, matching the RSI line.

***

### lineGradientOpacity?

```ts
optional lineGradientOpacity?: number;
```

Source: [types/src/index.ts:181](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L181)

Opacity of the gradient filled beneath the line-chart polyline, at its
strongest point. The fill uses `lineColor` and ramps to fully transparent at
the bottom of the price pane. Defaults to 0.28; set to 0 to disable the fill.

***

### lineTension?

```ts
optional lineTension?: number;
```

Source: [types/src/index.ts:191](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L191)

How much to round the line chart's corners, from 0 (straight segments
between closes) to 1 (fully smooth). Defaults to 0.

The curve is monotone-limited, so smoothing can never overshoot into a price
that didn't trade: every peak and trough stays on an actual close, and the
curve never leaves the range of the two closes it connects. Applies to the
gradient fill beneath the line as well, so the two stay flush.

***

### lineTipDot?

```ts
optional lineTipDot?: boolean;
```

Source: [types/src/index.ts:200](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L200)

Mark the line chart's newest end with a dot. Defaults to `true`.

Takes its color from `line` and its radius from `lineTipDotRadius`.

Only drawn in line mode, and it crossfades along with the line during a
candle↔line transition.

***

### lineTipDotRadius?

```ts
optional lineTipDotRadius?: number;
```

Source: [types/src/index.ts:206](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L206)

Radius of the `lineTipDot` in px. Defaults to 1.5× `lineWidth`, and never
less than 2px. The `lineTipPulse` ring scales with it: five sixths of the
radius wide, reaching 3.5× the radius at its widest.

***

### lineTipPulse?

```ts
optional lineTipPulse?: boolean;
```

Source: [types/src/index.ts:214](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L214)

Pulse a ring outward from the tip dot, once every 1.5s. Defaults to `false`.

Ignored when `lineTipDot` is off, and suppressed when the OS asks for reduced
motion. Because the ring never stops, enabling it keeps the chart repainting
continuously — leave it off for charts that should be able to go idle.

***

### lineTipPulseOpacity?

```ts
optional lineTipPulseOpacity?: number;
```

Source: [types/src/index.ts:219](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L219)

Opacity of the `lineTipPulse` ring as it leaves the dot, from 0 to 1. The
ring fades from here to fully transparent as it grows. Defaults to 0.4.

***

### lineWidth?

```ts
optional lineWidth?: number;
```

Source: [types/src/index.ts:175](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L175)

Line-chart-mode polyline stroke width in px, also the loading line's. Defaults to 1.5.

***

### minPriceRangeRatio?

```ts
optional minPriceRangeRatio?: number;
```

Source: [types/src/index.ts:281](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L281)

Narrowest auto-fit range as a fraction of the mid price (0.02 = ±1%); omit for none.

***

### priceDecimals?

```ts
optional priceDecimals?: number;
```

Source: [types/src/index.ts:279](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L279)

Fixed decimals for every price label and badge (0–12); omit for automatic.

***

### priceIndicatorBadgeText?

```ts
optional priceIndicatorBadgeText?: VroomColor;
```

Source: [types/src/index.ts:251](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L251)

Text on the current-price badge in the y-axis strip. Defaults to
`badgeText`. Useful when `accentBull` / `accentBear` are light enough that
the shared badge text doesn't read against them.

***

### priceIndicatorDashGap?

```ts
optional priceIndicatorDashGap?: number;
```

Source: [types/src/index.ts:233](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L233)

Gap in px between the current-price indicator's dashes. Defaults to 2; 0
draws a solid line.

Like `priceIndicatorDashLength`, this is the gap you see, caps included.

***

### priceIndicatorDashLength?

```ts
optional priceIndicatorDashLength?: number;
```

Source: [types/src/index.ts:226](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L226)

Dash length in px of the current-price indicator's dashed line. Defaults to 2.

This is the length you see, caps included: with `priceIndicatorRoundCap`
on, the round ends are taken out of it rather than added to it.

***

### priceIndicatorLineOpacity?

```ts
optional priceIndicatorLineOpacity?: number;
```

Source: [types/src/index.ts:243](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L243)

Opacity of the current-price indicator's dashed line, from 0 to 1. Defaults
to 1.

Multiplies the alpha of `accentBull` / `accentBear`, so a translucent accent
gets fainter still. The price badge keeps the accent's own alpha.

***

### priceIndicatorLineWidth?

```ts
optional priceIndicatorLineWidth?: number;
```

Source: [types/src/index.ts:235](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L235)

Stroke width in px of the current-price indicator's dashed line. Defaults to 1.

***

### priceIndicatorRoundCap?

```ts
optional priceIndicatorRoundCap?: boolean;
```

Source: [types/src/index.ts:245](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L245)

Round the ends of the current-price indicator's dashes. Defaults to `false`.

***

### showXAxis?

```ts
optional showXAxis?: boolean;
```

Source: [types/src/index.ts:277](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L277)

Show the time (x) axis strip along the bottom. Defaults to `true`.

Behaves like [VroomTheme.showYAxis](#showyaxis): the strip collapses, the plot
grows into it, and the time labels and crosshair time badge fade first.

***

### showYAxis?

```ts
optional showYAxis?: boolean;
```

Source: [types/src/index.ts:264](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L264)

Show the price (y) axis strip down the right edge. Defaults to `true`.

Hiding it collapses the strip and hands the reclaimed width to the plot, so
the candles widen to fill it. Its contents — price labels, the current-price
badge, the crosshair price badge, price-line axis badges — fade out ahead of
the collapse; gridlines and the price indicator's dotted line stay. Animated
over `transitionMs` unless the OS asks for reduced motion.

The axis drag-to-scale region goes with the strip, so a hidden axis can't be
scaled by dragging it.

***

### simpleCrosshairBadgeFontSize?

```ts
optional simpleCrosshairBadgeFontSize?: number;
```

Source: [types/src/index.ts:133](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L133)

Text size on the simple crosshair's capsule time badge, in CSS px; the
capsule scales with it. Omit for 15, or pass 0 to follow `badgeFontSize`.

***

### skeleton?

```ts
optional skeleton?: VroomColor;
```

Source: [types/src/index.ts:173](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L173)

The line drawn across the plot while loading (see the `loading` prop).
Defaults to inheriting `axisText`, the tone Liveline draws its loading
line in. The breath keeps it faint, so it still reads as structure
rather than as a series.

The line breathes between 14% and 30% of whatever color it ends up with,
on a cycle of about 2.4s. Any alpha given here multiplies into that, so
an opaque color is the usual choice. Pass `lineColor` to make the line
read as the chart's own series warming up instead, as Liveline does in
line mode.

***

### ~~volumeRadius?~~

```ts
optional volumeRadius?: number;
```

Source: [types/src/index.ts:110](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L110)

Corner radius (px) of the *top* of volume bars. Defaults to 0 (square).

#### Deprecated

Use `volume.radius`, which sits with the rest of the volume
styling. This still applies when `volume.radius` is omitted.

***

### wickBear?

```ts
optional wickBear?: VroomColor;
```

Source: [types/src/index.ts:97](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L97)

Down candle wick color. Defaults to the bear fill color.

***

### wickBull?

```ts
optional wickBull?: VroomColor;
```

Source: [types/src/index.ts:95](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L95)

Up candle wick color. Defaults to the bull fill color.

***

### wickRoundCap?

```ts
optional wickRoundCap?: boolean;
```

Source: [types/src/index.ts:103](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L103)

Round the wick end caps. Defaults to false.

***

### wickWidth?

```ts
optional wickWidth?: number;
```

Source: [types/src/index.ts:99](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L99)

Wick stroke width in px (applies to both up and down wicks). Defaults to 1.

***

### yAxisFade?

```ts
optional yAxisFade?: boolean;
```

Source: [types/src/index.ts:270](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L270)

Run the price series on under the y-axis strip, fading it out to the right
edge, instead of stopping it at the strip. Defaults to `false`. The labels
and badges stay drawn over it, and it goes with the strip as it hides.
