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

Source: [types/src/index.ts:711](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L711)

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

Source: [types/src/index.ts:763](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L763)

Cloud fill where Senkou A is below Senkou B. Default red.

---

### bullishCloudColor?

```ts
optional bullishCloudColor?: string | number;
```

Source: [types/src/index.ts:761](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L761)

Cloud fill where Senkou A is above Senkou B. Default green.

---

### chikouColor?

```ts
optional chikouColor?: string | number;
```

Source: [types/src/index.ts:752](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L752)

Chikou span color. Default teal.

---

### chikouVisible?

```ts
optional chikouVisible?: boolean;
```

Source: [types/src/index.ts:756](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L756)

Draw the Chikou span. Default true.

---

### chikouWidth?

```ts
optional chikouWidth?: number;
```

Source: [types/src/index.ts:754](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L754)

Chikou span stroke width in px. Default 1.

---

### cloudOpacity?

```ts
optional cloudOpacity?: number;
```

Source: [types/src/index.ts:765](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L765)

Cloud opacity 0..1, applied to whichever cloud color is in play. Default 0.15.

---

### cloudVisible?

```ts
optional cloudVisible?: boolean;
```

Source: [types/src/index.ts:759](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L759)

Draw the cloud between the two leading spans. Default true.

---

### displacement?

```ts
optional displacement?: number;
```

Source: [types/src/index.ts:725](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L725)

Bars the cloud leads by and Chikou lags by. Default 26, clamped to >= 0.
Changing it only moves what's already drawn — the lines themselves don't
recompute.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:713](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L713)

Draw the indicator. Default false.

---

### kijunColor?

```ts
optional kijunColor?: string | number;
```

Source: [types/src/index.ts:734](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L734)

Kijun-sen color. Default red.

---

### kijunPeriod?

```ts
optional kijunPeriod?: number;
```

Source: [types/src/index.ts:717](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L717)

Kijun-sen (base) lookback. Default 26, clamped to >= 1.

---

### kijunVisible?

```ts
optional kijunVisible?: boolean;
```

Source: [types/src/index.ts:738](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L738)

Draw the Kijun-sen. Default true.

---

### kijunWidth?

```ts
optional kijunWidth?: number;
```

Source: [types/src/index.ts:736](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L736)

Kijun-sen stroke width in px. Default 1.

---

### senkouAColor?

```ts
optional senkouAColor?: string | number;
```

Source: [types/src/index.ts:740](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L740)

Senkou Span A color. Default green.

---

### senkouAVisible?

```ts
optional senkouAVisible?: boolean;
```

Source: [types/src/index.ts:744](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L744)

Draw the Senkou Span A edge. Default true.

---

### senkouAWidth?

```ts
optional senkouAWidth?: number;
```

Source: [types/src/index.ts:742](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L742)

Senkou Span A stroke width in px. Default 1.

---

### senkouBColor?

```ts
optional senkouBColor?: string | number;
```

Source: [types/src/index.ts:746](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L746)

Senkou Span B color. Default orange.

---

### senkouBPeriod?

```ts
optional senkouBPeriod?: number;
```

Source: [types/src/index.ts:719](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L719)

Senkou Span B lookback. Default 52, clamped to >= 1.

---

### senkouBVisible?

```ts
optional senkouBVisible?: boolean;
```

Source: [types/src/index.ts:750](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L750)

Draw the Senkou Span B edge. Default true.

---

### senkouBWidth?

```ts
optional senkouBWidth?: number;
```

Source: [types/src/index.ts:748](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L748)

Senkou Span B stroke width in px. Default 1.

---

### tenkanColor?

```ts
optional tenkanColor?: string | number;
```

Source: [types/src/index.ts:728](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L728)

Tenkan-sen color (hex string or packed ARGB number). Default blue.

---

### tenkanPeriod?

```ts
optional tenkanPeriod?: number;
```

Source: [types/src/index.ts:715](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L715)

Tenkan-sen (conversion) lookback. Default 9, clamped to >= 1.

---

### tenkanVisible?

```ts
optional tenkanVisible?: boolean;
```

Source: [types/src/index.ts:732](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L732)

Draw the Tenkan-sen. Default true.

---

### tenkanWidth?

```ts
optional tenkanWidth?: number;
```

Source: [types/src/index.ts:730](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L730)

Tenkan-sen stroke width in px. Default 1.
