# Indicators

vroom ships seven indicator families. Two render in their own **pane below the
candles** (RSI, MACD); five are **overlays drawn on the price pane** (moving
averages, VWAP, Bollinger Bands, Ichimoku, Fair Value Gaps). Each is configured
through its own prop and is off until you enable it.

## RSI

Wilder's RSI in a pane below the candles, with configurable bands and an optional
moving-average trendline. See [`RSIConfig`](../reference/index.md).

```tsx
<VroomChart
  candles={candles}
  rsi={{ enabled: true, period: 14, upperBand: 70, lowerBand: 30, maVisible: true, maPeriod: 14 }}
/>
```

Defaults: `period` 14, `upperBand` 70, `lowerBand` 30, trendline on at period 14.
RSI reads closes only — Wilder's definition is built on close-to-close change —
so unlike the other averaged indicators it takes no `source`.

### Styling

The RSI line, the trendline, and the pair of dashed band rules each take their
own color, and the two lines share the same width scale as everywhere else.

```tsx
<VroomChart
  candles={candles}
  rsi={{
    enabled: true,
    maType: 'ema',
    lineColor: '#8957e5',
    lineWidth: 2,
    maColor: '#d29922',
    maWidth: 2,
    bandColor: '#30363d',
  }}
/>
```

Every style field is optional; leave one unset and the pane keeps its stock look.
Hide any part with `lineVisible`, `maVisible`, or `bandsVisible` — hiding the
rules takes their axis labels with them.

## MACD

MACD in its own pane below the candles: the gap between a fast and a slow moving
average, a signal line smoothing that gap, and a histogram of the distance
between the two. See [`MACDConfig`](../reference/index.md).

```tsx
<VroomChart candles={candles} macd={{ enabled: true, fast: 12, slow: 26, signal: 9 }} />
```

Defaults: `fast` 12, `slow` 26 (forced greater than `fast`), `signal` 9.

### Inputs

- `source` — price input ([`MASource`](../reference/index.md)) for the fast and
  slow legs: `close` (default), `open`, `high`, `low`, `hl2`, `hlc3`, `ohlc4`.
- `maType` — the averaging for both legs ([`MAKind`](../reference/index.md)):
  `'ema'` (default) or `'sma'`.
- `signalMaType` — same, for the signal line.

### Styling

Each series carries its own color, width, and visibility, and the histogram
takes four colors: one pair for bars above zero and one for bars below, each
split into a shade for bars growing away from zero and a lighter shade for bars
falling back toward it.

```tsx
<VroomChart
  candles={candles}
  macd={{
    enabled: true,
    lineColor: '#2962ff',
    lineWidth: 2,
    signalColor: '#ff6d00',
    signalWidth: 2,
    histogramUpColor: '#26a69a',
    histogramUpFadingColor: '#b2dfdb',
    histogramDownColor: '#ef5350',
    histogramDownFadingColor: '#ffcdd2',
    zeroLineColor: '#484f58',
  }}
/>
```

Every style field is optional. Leave the histogram colors unset and the bars
follow `theme.accentBull` / `theme.accentBear`; leave a fading color unset and it
derives from its base color at half opacity. Set all four histogram colors alike
for a flat, single-color histogram.

Hide any part with `lineVisible`, `signalVisible`, `histogramVisible`, or
`zeroLineVisible` — the pane rescales to fit whatever is left on show.

> When both RSI and MACD are enabled, both panes stack below the chart; the most
> recently enabled one is appended at the bottom.

## Moving averages (SMA / EMA)

Pass an array of overlay lines via `movingAverages` to draw a ribbon directly on
the price pane. Each entry is a [`MovingAverageOverlay`](../reference/index.md).

```tsx
<VroomChart
  candles={candles}
  movingAverages={[
    { maType: 'ema', period: 9, color: '#ffa726' },
    { maType: 'ema', period: 21, color: '#26c6da' },
    { maType: 'sma', period: 50, source: 'hlc3', width: 2 },
  ]}
/>
```

- `maType` — the averaging ([`MAKind`](../reference/index.md)): `'sma'` or `'ema'`.
- `period` — lookback in candles.
- `source` — price input ([`MASource`](../reference/index.md)): `close` (default),
  `open`, `high`, `low`, `hl2`, `hlc3`, `ohlc4`.
- `color` / `width` — line styling.

## VWAP

Session-anchored VWAP as a single line on the price pane, resetting each session.
See [`VWAPConfig`](../reference/index.md).

```tsx
<VroomChart
  candles={candles}
  vwap={{ enabled: true, resetMinutes: 0, color: '#00bcd4', width: 1.5 }}
/>
```

`resetMinutes` offsets the session boundary from UTC midnight (in minutes) — e.g.
pass `9 * 60 + 30` for a 09:30 UTC reset. The line breaks at each reset.

## Bollinger Bands

Three lines on the price pane — a basis moving average with an upper and lower
band ± N standard deviations away — plus a translucent fill between the bands.
See [`BollingerBandsConfig`](../reference/index.md).

```tsx
<VroomChart
  candles={candles}
  bollingerBands={{ enabled: true, period: 20, stdDev: 2 }}
/>
```

The formula: `middle = MA(source, period)`; `upper/lower = middle ± stdDev × σ`,
where σ is the **population** standard deviation of `source` over the same
trailing window. With `maType: 'ema'` the middle line becomes an EMA, but σ is
still computed around the window's arithmetic mean (the standard definition).

| Option | Default | Notes |
| --- | --- | --- |
| `period` | `20` | Lookback in candles, clamped to ≥ 1. |
| `stdDev` | `2` | Standard-deviation multiplier. |
| `source` | `'close'` | Any [`MASource`](../reference/index.md). |
| `maType` | `'sma'` | Basis-line averaging ([`MAKind`](../reference/index.md)). |
| `upperColor` / `lowerColor` | blue | Band line colors. |
| `middleColor` | orange | Basis line color. |
| `upperWidth` / `middleWidth` / `lowerWidth` | `1` | Stroke widths in px. |
| `fillVisible` | `true` | Translucent fill between the bands. |
| `fillOpacity` | `0.1` | 0..1, applied to the upper band color. |

The first `period − 1` candles have no value (the warmup window), so the lines
and fill start at the first fully-formed window.

## Ichimoku Cloud

Five lines on the price pane plus the cloud (kumo) shaded between the two
leading spans. See [`IchimokuConfig`](../reference/index.md).

```tsx
<VroomChart candles={candles} ichimoku={{ enabled: true }} />
```

| Line | Formula | Drawn |
| --- | --- | --- |
| Tenkan-sen | midpoint of the high/low range over `tenkanPeriod` | on the bar |
| Kijun-sen | same over `kijunPeriod` | on the bar |
| Senkou Span A | `(tenkan + kijun) / 2` | `displacement` bars ahead |
| Senkou Span B | midpoint of the high/low range over `senkouBPeriod` | `displacement` bars ahead |
| Chikou span | the close | `displacement` bars behind |

The cloud fills between the two leading spans, tinted green where span A is
above span B and red where it is below. The two tones meet exactly at each
crossover.

### Drawing past the newest candle

The leading spans are plotted 26 bars into the future by default, where no
candle exists yet. That is the point of them — the cloud is a forecast of
support and resistance — so the chart reserves that much empty time on the
right when it frames itself, and enabling the indicator later pulls the view
forward to match. You do not need to pan to see the forward cloud.

If you drive the viewport yourself with `visibleRange`, leave room for it: the
cloud's right edge sits at `lastCandle.timeMs + displacement × interval`.

Like every other price-pane overlay, Ichimoku's values don't feed the automatic
y-axis fit, which frames the candles alone — so on a strongly trending chart the
cloud can run off the top or bottom of the pane.

### Options

| Option | Default | Notes |
| --- | --- | --- |
| `tenkanPeriod` | `9` | Conversion-line lookback, clamped to ≥ 1. |
| `kijunPeriod` | `26` | Base-line lookback, clamped to ≥ 1. |
| `senkouBPeriod` | `52` | Span B lookback, clamped to ≥ 1. |
| `displacement` | `26` | Bars the cloud leads and Chikou lags by, clamped to ≥ 0. |

Changing `displacement` only moves what's already drawn — the lines don't
recompute — so it's cheap to animate or bind to a slider.

### Styling

Each of the five lines takes its own color, width, and visibility, and the cloud
takes a color per direction plus an opacity.

```tsx
<VroomChart
  candles={candles}
  ichimoku={{
    enabled: true,
    tenkanColor: '#2962ff',
    tenkanWidth: 1,
    kijunColor: '#ef5350',
    senkouAColor: '#26a69a',
    senkouBColor: '#ff6d00',
    chikouColor: '#00bcd4',
    bullishCloudColor: '#26a69a',
    bearishCloudColor: '#ef5350',
    cloudOpacity: 0.15,
  }}
/>
```

Every style field is optional; leave one unset and it keeps its stock look. Hide
any part with `tenkanVisible`, `kijunVisible`, `senkouAVisible`,
`senkouBVisible`, `chikouVisible`, or `cloudVisible` — the cloud and the span
edges are independent, so you can shade the cloud without stroking its borders.

Each line starts at the first fully-formed window of its own lookback, so span B
(52 bars by default) begins latest.

## Fair Value Gaps

Shaded boxes over three-candle imbalances — a run so fast the first and third
candles' wicks never overlap, leaving a band of price that was skipped. See
[`FairValueGapsConfig`](../reference/index.md).

```tsx
<VroomChart candles={candles} fairValueGaps={{ enabled: true }} />
```

A gap forms around the middle bar of the three:

| Direction | Condition | Box spans |
| --- | --- | --- |
| Bullish | `candles[i - 1].high < candles[i + 1].low` | that untouched range, shaded green |
| Bearish | `candles[i - 1].low > candles[i + 1].high` | that untouched range, shaded red |

Each box is anchored to the middle bar's open and runs `boxLength` bars to the
right. Boxes draw behind the candles, so the bars that formed the imbalance stay
readable over their own shading, and their labels draw in front.

### Filling

A gap is *filled* once price trades back through it — down to the bottom of a
bullish gap, up to the top of a bearish one. `fillType` picks which price
settles that: `'close'` (the default) needs a candle to close past the far edge,
while `'wick'` settles it the moment a high or low reaches through.

What happens next is `deleteAfterFill`. On by default, so a filled gap
disappears and only live imbalances remain. Turn it off and the box stays but
stops at the bar that filled it, leaving a record of where the rebalance
happened.

Filling is always tracked, so toggling `deleteAfterFill` or switching `fillType`
costs nothing but a redraw.

### Inversion

Closing through a gap doesn't have to be the end of it. With `showInverse`, the
box carries on with its polarity flipped: the band price rejected on the way
through becomes a zone of the opposite kind, and it stays on the chart until
price reclaims it.

```tsx
<VroomChart candles={candles} fairValueGaps={{ enabled: true, showInverse: true }} />
```

| Was | Becomes | Ends when |
| --- | --- | --- |
| Bullish gap, closed below | bearish resistance overhead | a close back above its top |
| Bearish gap, closed above | bullish support underneath | a close back below its bottom |

Invalidation mirrors the fill exactly, `fillType` and all, so under `'wick'` a
high or low reaching the far edge is enough to end the inversion too.

The inverse box starts where the original one stops — the close of the bar that
filled the gap — so the two never overlap, and it measures its own `boxLength`
from there. Colors come from `inverseBullishColor` / `inverseBearishColor`,
which describe the polarity of the *zone*, not the gap: a violated bullish gap
is a bearish zone, so `inverseBearishColor` is what paints it. Labels read
`'iFVG'` unless you set `inverseLabel`.

`deleteAfterFill` still governs only the original box, which makes the default
pairing read well — the original vanishes at the fill and the inverse picks up
from there. Turn `deleteAfterFill` off and you see both halves of the story: the
gap up to its fill, then the zone it turned into.

### Options

| Option | Default | Notes |
| --- | --- | --- |
| `maxBarsBack` | `300` | Bars to scan, clamped to ≥ 0. `0` finds nothing. |
| `waitForClose` | `false` | Withhold a gap until its third candle closes. |
| `fillType` | `'close'` | `'close'` or `'wick'`. |
| `deleteAfterFill` | `true` | Hide a filled gap, rather than truncating it. |
| `extendBoxes` | `false` | Run boxes to the newest bar instead of `boxLength`. |
| `boxLength` | `20` | Box width in bars, clamped to ≥ 1. |
| `showInverse` | `false` | Keep a filled gap on the chart with its polarity flipped. |

With `waitForClose` off, a gap formed by the still-forming bar appears
immediately — and disappears again if that bar fills back in. Turn it on if you
only want confirmed setups.

Only these four options rescan the series: `enabled`, `maxBarsBack`,
`waitForClose` and `fillType`. Box geometry, colors, labels and `showInverse`
are all applied when drawing, so they're cheap to bind to a control.

### Styling

```tsx
<VroomChart
  candles={candles}
  fairValueGaps={{
    enabled: true,
    bullishColor: '#26a69a',
    bearishColor: '#ff9800',
    opacity: 0.15,
    borderStyle: 'dashed',
    borderWidth: 1,
    label: 'FVG',
  }}
/>
```

The fill takes a color per direction plus a shared `opacity`. The outline is
separate: hide it with `borderVisible`, style it `'solid'`, `'dotted'` or
`'dashed'`, and override its color per direction with `bullishBorderColor` /
`bearishBorderColor` — both of which default to the matching fill color, so
restyling only the fill keeps the outline in the same hue.

Labels are on by default and read `'FVG'`. A fixed-length box carries its label
inside its right end; an extended one puts it out in the empty slots past the
newest candle, `labelDistance` bars clear of it. Set `labelColor` to override
the border color it otherwise inherits, and `labelFontSize` to override the axis
font size.

Like every other price-pane overlay, gap boxes don't feed the automatic y-axis
fit, which frames the candles alone.
