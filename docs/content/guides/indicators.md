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

MACD (fast/slow/signal EMAs) in its own pane below the candles. See
[`MACDConfig`](../reference/index.md).

```tsx
<VroomChart candles={candles} macd={{ enabled: true, fast: 12, slow: 26, signal: 9 }} />
```

Defaults: `fast` 12, `slow` 26 (forced greater than `fast`), `signal` 9.

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
