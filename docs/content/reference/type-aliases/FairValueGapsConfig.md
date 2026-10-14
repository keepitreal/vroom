# `FairValueGapsConfig`

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

Source: [types/src/index.ts:757](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L757)

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

Source: [types/src/index.ts:803](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L803)

Outline color for bearish gaps. Defaults to `bearishColor` at full alpha.

---

### bearishColor?

```ts
optional bearishColor?: VroomColor;
```

Source: [types/src/index.ts:790](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L790)

Fill color for bearish gaps. Default red.

---

### borderStyle?

```ts
optional borderStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:797](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L797)

Outline style. Default `'solid'`.

---

### borderVisible?

```ts
optional borderVisible?: boolean;
```

Source: [types/src/index.ts:795](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L795)

Draw the box outline. Default true.

---

### borderWidth?

```ts
optional borderWidth?: number;
```

Source: [types/src/index.ts:799](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L799)

Outline stroke width in px. Default 1.

---

### boxLength?

```ts
optional boxLength?: number;
```

Source: [types/src/index.ts:785](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L785)

Box width in bars when `extendBoxes` is off. Default 20, clamped to >= 1.

---

### bullishBorderColor?

```ts
optional bullishBorderColor?: VroomColor;
```

Source: [types/src/index.ts:801](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L801)

Outline color for bullish gaps. Defaults to `bullishColor` at full alpha.

---

### bullishColor?

```ts
optional bullishColor?: VroomColor;
```

Source: [types/src/index.ts:788](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L788)

Fill color for bullish gaps (hex string or packed ARGB). Default green.

---

### deleteAfterFill?

```ts
optional deleteAfterFill?: boolean;
```

Source: [types/src/index.ts:778](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L778)

Hide a gap once it's been filled. Default true. When false the box stays
but stops at the bar that filled it, leaving a record of the rebalance.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:759](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L759)

Draw the indicator. Default false.

---

### extendBoxes?

```ts
optional extendBoxes?: boolean;
```

Source: [types/src/index.ts:783](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L783)

Run every box to the right edge of the pane instead of ending it after
`boxLength` bars. Default false. A filled box still stops at its fill bar.

---

### fillType?

```ts
optional fillType?: "close" | "wick";
```

Source: [types/src/index.ts:773](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L773)

Which price counts as trading back through the gap. `'close'` (default)
needs a candle to close past the far edge; `'wick'` settles it the moment
a high or low reaches through.

---

### inverseBearishColor?

```ts
optional inverseBearishColor?: VroomColor;
```

Source: [types/src/index.ts:841](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L841)

Fill color for inverted zones that are bearish — that is, for _bullish_
gaps price has broken below. Defaults to `bearishColor`.

---

### inverseBullishColor?

```ts
optional inverseBullishColor?: VroomColor;
```

Source: [types/src/index.ts:836](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L836)

Fill color for inverted zones that are bullish — that is, for _bearish_
gaps price has broken above. Defaults to `bullishColor`.

---

### inverseLabel?

```ts
optional inverseLabel?: string;
```

Source: [types/src/index.ts:843](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L843)

Label text on inverted boxes. Default `'iFVG'`.

---

### label?

```ts
optional label?: string;
```

Source: [types/src/index.ts:808](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L808)

Label text. Default `'FVG'`.

---

### labelColor?

```ts
optional labelColor?: VroomColor;
```

Source: [types/src/index.ts:816](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L816)

Label color. Defaults to the box's border color.

---

### labelDistance?

```ts
optional labelDistance?: number;
```

Source: [types/src/index.ts:814](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L814)

Bars of clearance between the box and its label, used only under
`extendBoxes` — a fixed-length box places the label inside its right end.
Default 10, clamped to >= 0.

---

### labelFontSize?

```ts
optional labelFontSize?: number;
```

Source: [types/src/index.ts:818](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L818)

Label font size in px. Defaults to the axis font size.

---

### maxBarsBack?

```ts
optional maxBarsBack?: number;
```

Source: [types/src/index.ts:761](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L761)

How many bars back to scan for gaps. Default 300, clamped to >= 0.

---

### opacity?

```ts
optional opacity?: number;
```

Source: [types/src/index.ts:792](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L792)

Fill opacity 0..1, applied to whichever fill color is in play. Default 0.15.

---

### showInverse?

```ts
optional showInverse?: boolean;
```

Source: [types/src/index.ts:831](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L831)

Keep drawing a gap after it's been filled, with its polarity flipped — the
band price rejected on the way through becomes a zone of the opposite
kind. Default false.

The inverse box starts where the original one stops, at the close of the
bar that filled the gap, and lasts until price reclaims the band the other
way (by the same rule `fillType` sets). This pairs with the default
`deleteAfterFill: true`: the original box vanishes at the fill and the
inverse takes over from there.

---

### showLabels?

```ts
optional showLabels?: boolean;
```

Source: [types/src/index.ts:806](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L806)

Draw a text label on each box. Default true.

---

### waitForClose?

```ts
optional waitForClose?: boolean;
```

Source: [types/src/index.ts:767](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L767)

Withhold a gap until its third candle closes. Default false, so a gap
formed by the still-forming bar appears immediately and disappears again
if that bar fills back in.
