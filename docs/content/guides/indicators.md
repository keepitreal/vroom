# Indicators

vroom ships five indicator families. Two render in their own **pane below the
candles** (RSI, MACD); three are **overlays drawn on the price pane** (moving
averages, VWAP, Bollinger Bands). Each is configured through its own prop and
is off until you enable it.

## RSI

Wilder's RSI in a pane below the candles, with configurable bands and an optional
moving-average trendline. See [`RSIConfig`](../reference/index.md).

```tsx
<VroomChart
  candles={candles}
  rsi={{ enabled: true, period: 14, upperBand: 70, lowerBand: 30, maEnabled: true, maPeriod: 14 }}
/>
```

Defaults: `period` 14, `upperBand` 70, `lowerBand` 30, trendline on at length 14.

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
- `maType` — `'ema'` (default) or `'sma'` for both legs.
- `signalMaType` — `'ema'` (default) or `'sma'` for the signal line.

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
    macdColor: '#2962ff',
    macdWidth: 2,
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

Hide any part with `macdVisible`, `signalVisible`, `histogramVisible`, or
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
    { kind: 'ema', length: 9, color: '#ffa726' },
    { kind: 'ema', length: 21, color: '#26c6da' },
    { kind: 'sma', length: 50, source: 'hlc3', width: 2 },
  ]}
/>
```

- `kind` — `'sma'` or `'ema'`.
- `length` — lookback in candles.
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
trailing window. With `basis: 'ema'` the middle line becomes an EMA, but σ is
still computed around the window's arithmetic mean (the standard definition).

| Option | Default | Notes |
| --- | --- | --- |
| `period` | `20` | Lookback in candles, clamped to ≥ 1. |
| `stdDev` | `2` | Standard-deviation multiplier. |
| `source` | `'close'` | Any [`MASource`](../reference/index.md). |
| `basis` | `'sma'` | `'sma'` or `'ema'` basis line. |
| `upperColor` / `lowerColor` | blue | Band line colors. |
| `middleColor` | orange | Basis line color. |
| `upperWidth` / `middleWidth` / `lowerWidth` | `1` | Stroke widths in px. |
| `fill` | `true` | Translucent fill between the bands. |
| `fillOpacity` | `0.1` | 0..1, applied to the upper band color. |

The first `period − 1` candles have no value (the warmup window), so the lines
and fill start at the first fully-formed window.
