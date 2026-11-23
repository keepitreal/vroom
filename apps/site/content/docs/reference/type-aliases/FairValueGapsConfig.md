---
title: "FairValueGapsConfig"
description: "Fair Value Gap overlay config."
---

```ts
type FairValueGapsConfig = {
  bearishBorderColor?: VroomColor;
  bearishColor?: VroomColor;
  borderStyle?: "solid" | "dotted" | "dashed";
  borderVisible?: boolean;
  borderWidth?: number;
  boxLength?: number;
  bullishBorderColor?: VroomColor;
  bullishColor?: VroomColor;
  deleteAfterFill?: boolean;
  enabled?: boolean;
  extendBoxes?: boolean;
  fillType?: "close" | "wick";
  inverseBearishColor?: VroomColor;
  inverseBullishColor?: VroomColor;
  inverseLabel?: string;
  label?: string;
  labelColor?: VroomColor;
  labelDistance?: number;
  labelFontSize?: number;
  maxBarsBack?: number;
  opacity?: number;
  showInverse?: boolean;
  showLabels?: boolean;
  waitForClose?: boolean;
};
```

Source: [types/src/index.ts:877](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L877)

Fair Value Gap overlay config. Shaded boxes on the price pane marking
three-candle imbalances — a run so fast the first and third candles' wicks
never overlap, leaving a band of price that was skipped.

A gap is bullish when `candles[i - 1].high < candles[i + 1].low` and bearish
when `candles[i - 1].low > candles[i + 1].high`, and spans the untouched
range between those two wicks. Each box is anchored to the middle bar's open
and runs `boxLength` bars to the right, or to the pane's edge under
`extendBoxes`.

Gaps are tracked until price trades back through them — see `fillType` for
which price settles that, and `deleteAfterFill` for what happens once it
does. With `showInverse`, a filled gap carries on as a zone of the opposite
polarity. Unlike the line overlays, the boxes are pure geometry: they don't
feed the automatic y-axis fit.

## Properties

### bearishBorderColor?

```ts
optional bearishBorderColor?: VroomColor;
```

Source: [types/src/index.ts:923](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L923)

Outline color for bearish gaps. Defaults to `bearishColor` at full alpha.

***

### bearishColor?

```ts
optional bearishColor?: VroomColor;
```

Source: [types/src/index.ts:910](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L910)

Fill color for bearish gaps. Default red.

***

### borderStyle?

```ts
optional borderStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:917](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L917)

Outline style. Default `'solid'`.

***

### borderVisible?

```ts
optional borderVisible?: boolean;
```

Source: [types/src/index.ts:915](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L915)

Draw the box outline. Default true.

***

### borderWidth?

```ts
optional borderWidth?: number;
```

Source: [types/src/index.ts:919](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L919)

Outline stroke width in px. Default 1.

***

### boxLength?

```ts
optional boxLength?: number;
```

Source: [types/src/index.ts:905](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L905)

Box width in bars when `extendBoxes` is off. Default 20, clamped to >= 1.

***

### bullishBorderColor?

```ts
optional bullishBorderColor?: VroomColor;
```

Source: [types/src/index.ts:921](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L921)

Outline color for bullish gaps. Defaults to `bullishColor` at full alpha.

***

### bullishColor?

```ts
optional bullishColor?: VroomColor;
```

Source: [types/src/index.ts:908](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L908)

Fill color for bullish gaps (hex string or packed ARGB). Default green.

***

### deleteAfterFill?

```ts
optional deleteAfterFill?: boolean;
```

Source: [types/src/index.ts:898](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L898)

Hide a gap once it's been filled. Default true. When false the box stays
but stops at the bar that filled it, leaving a record of the rebalance.

***

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:879](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L879)

Draw the indicator. Default false.

***

### extendBoxes?

```ts
optional extendBoxes?: boolean;
```

Source: [types/src/index.ts:903](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L903)

Run every box to the right edge of the pane instead of ending it after
`boxLength` bars. Default false. A filled box still stops at its fill bar.

***

### fillType?

```ts
optional fillType?: "close" | "wick";
```

Source: [types/src/index.ts:893](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L893)

Which price counts as trading back through the gap. `'close'` (default)
needs a candle to close past the far edge; `'wick'` settles it the moment
a high or low reaches through.

***

### inverseBearishColor?

```ts
optional inverseBearishColor?: VroomColor;
```

Source: [types/src/index.ts:961](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L961)

Fill color for inverted zones that are bearish — that is, for *bullish*
gaps price has broken below. Defaults to `bearishColor`.

***

### inverseBullishColor?

```ts
optional inverseBullishColor?: VroomColor;
```

Source: [types/src/index.ts:956](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L956)

Fill color for inverted zones that are bullish — that is, for *bearish*
gaps price has broken above. Defaults to `bullishColor`.

***

### inverseLabel?

```ts
optional inverseLabel?: string;
```

Source: [types/src/index.ts:963](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L963)

Label text on inverted boxes. Default `'iFVG'`.

***

### label?

```ts
optional label?: string;
```

Source: [types/src/index.ts:928](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L928)

Label text. Default `'FVG'`.

***

### labelColor?

```ts
optional labelColor?: VroomColor;
```

Source: [types/src/index.ts:936](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L936)

Label color. Defaults to the box's border color.

***

### labelDistance?

```ts
optional labelDistance?: number;
```

Source: [types/src/index.ts:934](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L934)

Bars of clearance between the box and its label, used only under
`extendBoxes` — a fixed-length box places the label inside its right end.
Default 10, clamped to >= 0.

***

### labelFontSize?

```ts
optional labelFontSize?: number;
```

Source: [types/src/index.ts:938](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L938)

Label font size in px. Defaults to the axis font size.

***

### maxBarsBack?

```ts
optional maxBarsBack?: number;
```

Source: [types/src/index.ts:881](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L881)

How many bars back to scan for gaps. Default 300, clamped to >= 0.

***

### opacity?

```ts
optional opacity?: number;
```

Source: [types/src/index.ts:912](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L912)

Fill opacity 0..1, applied to whichever fill color is in play. Default 0.15.

***

### showInverse?

```ts
optional showInverse?: boolean;
```

Source: [types/src/index.ts:951](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L951)

Keep drawing a gap after it's been filled, with its polarity flipped — the
band price rejected on the way through becomes a zone of the opposite
kind. Default false.

The inverse box starts where the original one stops, at the close of the
bar that filled the gap, and lasts until price reclaims the band the other
way (by the same rule `fillType` sets). This pairs with the default
`deleteAfterFill: true`: the original box vanishes at the fill and the
inverse takes over from there.

***

### showLabels?

```ts
optional showLabels?: boolean;
```

Source: [types/src/index.ts:926](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L926)

Draw a text label on each box. Default true.

***

### waitForClose?

```ts
optional waitForClose?: boolean;
```

Source: [types/src/index.ts:887](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L887)

Withhold a gap until its third candle closes. Default false, so a gap
formed by the still-forming bar appears immediately and disappears again
if that bar fills back in.
