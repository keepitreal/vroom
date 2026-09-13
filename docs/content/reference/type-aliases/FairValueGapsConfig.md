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
  label?: string;
  labelColor?: VroomColor;
  labelDistance?: number;
  labelFontSize?: number;
  maxBarsBack?: number;
  opacity?: number;
  showLabels?: boolean;
  waitForClose?: boolean;
};
```

Source: [types/src/index.ts:669](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L669)

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
does. Unlike the line overlays, the boxes are pure geometry: they don't feed
the automatic y-axis fit.

## Properties

### bearishBorderColor?

```ts
optional bearishBorderColor?: VroomColor;
```

Source: [types/src/index.ts:715](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L715)

Outline color for bearish gaps. Defaults to `bearishColor` at full alpha.

---

### bearishColor?

```ts
optional bearishColor?: VroomColor;
```

Source: [types/src/index.ts:702](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L702)

Fill color for bearish gaps. Default red.

---

### borderStyle?

```ts
optional borderStyle?: "solid" | "dotted" | "dashed";
```

Source: [types/src/index.ts:709](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L709)

Outline style. Default `'solid'`.

---

### borderVisible?

```ts
optional borderVisible?: boolean;
```

Source: [types/src/index.ts:707](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L707)

Draw the box outline. Default true.

---

### borderWidth?

```ts
optional borderWidth?: number;
```

Source: [types/src/index.ts:711](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L711)

Outline stroke width in px. Default 1.

---

### boxLength?

```ts
optional boxLength?: number;
```

Source: [types/src/index.ts:697](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L697)

Box width in bars when `extendBoxes` is off. Default 20, clamped to >= 1.

---

### bullishBorderColor?

```ts
optional bullishBorderColor?: VroomColor;
```

Source: [types/src/index.ts:713](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L713)

Outline color for bullish gaps. Defaults to `bullishColor` at full alpha.

---

### bullishColor?

```ts
optional bullishColor?: VroomColor;
```

Source: [types/src/index.ts:700](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L700)

Fill color for bullish gaps (hex string or packed ARGB). Default green.

---

### deleteAfterFill?

```ts
optional deleteAfterFill?: boolean;
```

Source: [types/src/index.ts:690](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L690)

Hide a gap once it's been filled. Default true. When false the box stays
but stops at the bar that filled it, leaving a record of the rebalance.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:671](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L671)

Draw the indicator. Default false.

---

### extendBoxes?

```ts
optional extendBoxes?: boolean;
```

Source: [types/src/index.ts:695](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L695)

Run every box to the right edge of the pane instead of ending it after
`boxLength` bars. Default false. A filled box still stops at its fill bar.

---

### fillType?

```ts
optional fillType?: "close" | "wick";
```

Source: [types/src/index.ts:685](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L685)

Which price counts as trading back through the gap. `'close'` (default)
needs a candle to close past the far edge; `'wick'` settles it the moment
a high or low reaches through.

---

### label?

```ts
optional label?: string;
```

Source: [types/src/index.ts:720](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L720)

Label text. Default `'FVG'`.

---

### labelColor?

```ts
optional labelColor?: VroomColor;
```

Source: [types/src/index.ts:728](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L728)

Label color. Defaults to the box's border color.

---

### labelDistance?

```ts
optional labelDistance?: number;
```

Source: [types/src/index.ts:726](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L726)

Bars of clearance between the box and its label, used only under
`extendBoxes` — a fixed-length box places the label inside its right end.
Default 10, clamped to >= 0.

---

### labelFontSize?

```ts
optional labelFontSize?: number;
```

Source: [types/src/index.ts:730](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L730)

Label font size in px. Defaults to the axis font size.

---

### maxBarsBack?

```ts
optional maxBarsBack?: number;
```

Source: [types/src/index.ts:673](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L673)

How many bars back to scan for gaps. Default 300, clamped to >= 0.

---

### opacity?

```ts
optional opacity?: number;
```

Source: [types/src/index.ts:704](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L704)

Fill opacity 0..1, applied to whichever fill color is in play. Default 0.15.

---

### showLabels?

```ts
optional showLabels?: boolean;
```

Source: [types/src/index.ts:718](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L718)

Draw a text label on each box. Default true.

---

### waitForClose?

```ts
optional waitForClose?: boolean;
```

Source: [types/src/index.ts:679](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L679)

Withhold a gap until its third candle closes. Default false, so a gap
formed by the still-forming bar appears immediately and disappears again
if that bar fills back in.
