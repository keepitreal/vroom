# `IchimokuConfig`

```ts
type IchimokuConfig = {
  bearishCloudColor?: string | number;
  bullishCloudColor?: string | number;
  chikouColor?: string | number;
  chikouVisible?: boolean;
  chikouWidth?: number;
  cloudOpacity?: number;
  cloudVisible?: boolean;
  displacement?: number;
  enabled?: boolean;
  kijunColor?: string | number;
  kijunPeriod?: number;
  kijunVisible?: boolean;
  kijunWidth?: number;
  senkouAColor?: string | number;
  senkouAVisible?: boolean;
  senkouAWidth?: number;
  senkouBColor?: string | number;
  senkouBPeriod?: number;
  senkouBVisible?: boolean;
  senkouBWidth?: number;
  tenkanColor?: string | number;
  tenkanPeriod?: number;
  tenkanVisible?: boolean;
  tenkanWidth?: number;
};
```

Source: [types/src/index.ts:622](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L622)

Ichimoku Kinko Hyo overlay config. Five lines on the price pane plus the
cloud (kumo) shaded between the two leading spans. No pane is reserved.

Unlike the other overlays, three of the lines are drawn away from the bar
they were computed on:

- Senkou A and B lead by `displacement` bars, so the cloud extends past the
  newest candle into empty time. The chart reserves that space when it frames
  itself, so the forward cloud is on screen without panning.
- Chikou lags by `displacement` bars.

Ichimoku is built on highs and lows rather than a single price series, so it
takes no [MASource](MASource.md) or [MAKind](MAKind.md).

Like the other price-pane overlays, its values don't feed the automatic
y-axis fit — the cloud can run off the top or bottom of the pane on a chart
scaled to the candles alone.

## Properties

### bearishCloudColor?

```ts
optional bearishCloudColor?: string | number;
```

Source: [types/src/index.ts:674](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L674)

Cloud fill where Senkou A is below Senkou B. Default red.

---

### bullishCloudColor?

```ts
optional bullishCloudColor?: string | number;
```

Source: [types/src/index.ts:672](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L672)

Cloud fill where Senkou A is above Senkou B. Default green.

---

### chikouColor?

```ts
optional chikouColor?: string | number;
```

Source: [types/src/index.ts:663](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L663)

Chikou span color. Default teal.

---

### chikouVisible?

```ts
optional chikouVisible?: boolean;
```

Source: [types/src/index.ts:667](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L667)

Draw the Chikou span. Default true.

---

### chikouWidth?

```ts
optional chikouWidth?: number;
```

Source: [types/src/index.ts:665](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L665)

Chikou span stroke width in px. Default 1.

---

### cloudOpacity?

```ts
optional cloudOpacity?: number;
```

Source: [types/src/index.ts:676](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L676)

Cloud opacity 0..1, applied to whichever cloud color is in play. Default 0.15.

---

### cloudVisible?

```ts
optional cloudVisible?: boolean;
```

Source: [types/src/index.ts:670](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L670)

Draw the cloud between the two leading spans. Default true.

---

### displacement?

```ts
optional displacement?: number;
```

Source: [types/src/index.ts:636](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L636)

Bars the cloud leads by and Chikou lags by. Default 26, clamped to >= 0.
Changing it only moves what's already drawn — the lines themselves don't
recompute.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:624](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L624)

Draw the indicator. Default false.

---

### kijunColor?

```ts
optional kijunColor?: string | number;
```

Source: [types/src/index.ts:645](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L645)

Kijun-sen color. Default red.

---

### kijunPeriod?

```ts
optional kijunPeriod?: number;
```

Source: [types/src/index.ts:628](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L628)

Kijun-sen (base) lookback. Default 26, clamped to >= 1.

---

### kijunVisible?

```ts
optional kijunVisible?: boolean;
```

Source: [types/src/index.ts:649](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L649)

Draw the Kijun-sen. Default true.

---

### kijunWidth?

```ts
optional kijunWidth?: number;
```

Source: [types/src/index.ts:647](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L647)

Kijun-sen stroke width in px. Default 1.

---

### senkouAColor?

```ts
optional senkouAColor?: string | number;
```

Source: [types/src/index.ts:651](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L651)

Senkou Span A color. Default green.

---

### senkouAVisible?

```ts
optional senkouAVisible?: boolean;
```

Source: [types/src/index.ts:655](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L655)

Draw the Senkou Span A edge. Default true.

---

### senkouAWidth?

```ts
optional senkouAWidth?: number;
```

Source: [types/src/index.ts:653](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L653)

Senkou Span A stroke width in px. Default 1.

---

### senkouBColor?

```ts
optional senkouBColor?: string | number;
```

Source: [types/src/index.ts:657](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L657)

Senkou Span B color. Default orange.

---

### senkouBPeriod?

```ts
optional senkouBPeriod?: number;
```

Source: [types/src/index.ts:630](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L630)

Senkou Span B lookback. Default 52, clamped to >= 1.

---

### senkouBVisible?

```ts
optional senkouBVisible?: boolean;
```

Source: [types/src/index.ts:661](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L661)

Draw the Senkou Span B edge. Default true.

---

### senkouBWidth?

```ts
optional senkouBWidth?: number;
```

Source: [types/src/index.ts:659](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L659)

Senkou Span B stroke width in px. Default 1.

---

### tenkanColor?

```ts
optional tenkanColor?: string | number;
```

Source: [types/src/index.ts:639](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L639)

Tenkan-sen color (hex string or packed ARGB number). Default blue.

---

### tenkanPeriod?

```ts
optional tenkanPeriod?: number;
```

Source: [types/src/index.ts:626](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L626)

Tenkan-sen (conversion) lookback. Default 9, clamped to >= 1.

---

### tenkanVisible?

```ts
optional tenkanVisible?: boolean;
```

Source: [types/src/index.ts:643](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L643)

Draw the Tenkan-sen. Default true.

---

### tenkanWidth?

```ts
optional tenkanWidth?: number;
```

Source: [types/src/index.ts:641](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L641)

Tenkan-sen stroke width in px. Default 1.
