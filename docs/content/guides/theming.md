# Theming

Style the chart through the `theme` prop. Every field is optional — anything you
omit keeps the library default. Most fields are **colors**; a few are numeric or
boolean knobs for wick width and rounded candle/volume shapes.

```tsx
<VroomChart
  candles={candles}
  theme={{
    // colors
    background: '#0d1117',
    bull: '#26a69a',
    bear: '#ef5350',
    accentBull: '#26a69a',
    accentBear: '#ef5350',
    borderBull: '#1b7a70',
    borderBear: '#c0392b',
    wickBull: '#26a69a',
    wickBear: '#ef5350',
    grid: '#1f2630',
    axisText: '#8b949e',
    crosshair: '#c9d1d9',
    crosshairTarget: '#ffffff',
    // shape & size
    wickWidth: 1,
    wickRoundCap: true,
    candleRadius: 2,
    volumeRadius: 2,
  }}
/>
```

## Color formats

A color ([`VroomColor`](../reference/index.md)) is either:

- a **hex string** — `'#0d1117'` (6-digit RGB) or `'#ff0d1117'` (8-digit
  `#aarrggbb` with alpha), or
- a **packed ARGB number** — e.g. `0xff0d1117`.

## Colors

| Field | Affects |
| --- | --- |
| `background` | Chart + axis-strip background |
| `bull` | Up candle body fill |
| `bear` | Down candle body fill |
| `borderBull` | Up candle body border (see [Candle borders](#candle-borders)) |
| `borderBear` | Down candle body border (see [Candle borders](#candle-borders)) |
| `wickBull` | Up candle wick (defaults to `bull`) |
| `wickBear` | Down candle wick (defaults to `bear`) |
| `accentBull` | Generic up color: rising price indicator, up volume bars, MACD histogram |
| `accentBear` | Generic down color: falling price indicator, down volume bars, MACD histogram |
| `grid` | Gridlines |
| `axisText` | Price + time axis label text |
| `crosshair` | Crosshair dashed lines |
| `crosshairTarget` | The hollow ring/dot at the crosshair intersection |

## Shape & size

These control candle geometry. All default to off (square bodies/bars, 1px butt-cap
wicks), so charts are unchanged until you set them.

| Field | Type | Effect |
| --- | --- | --- |
| `wickWidth` | number (px) | Wick stroke width, for both up and down wicks. Default `1`. |
| `wickRoundCap` | boolean | Round the wick end caps. Default `false`. |
| `candleRadius` | number (px) | Corner radius of candle bodies (all four corners). Default `0` (square). |
| `volumeRadius` | number (px) | Corner radius of the **top** corners of volume bars. Default `0` (square). |

Radii are clamped to the shape they round, so thin candles and short volume bars
degrade gracefully instead of over-rounding.

## Candle borders

`borderBull` / `borderBear` draw a 1px outline **inside** the candle body. Drawing
it inside means a contrasting border never changes the candle's width, and hiding
it leaves the candle exactly its normal size.

- **Hidden (default):** omit the field, or set it to the body fill color. The
  border isn't drawn.
- **Visible:** set a contrasting color (e.g. `borderBull: '#ffffff'`) to outline
  the body from within.

See [`VroomTheme`](../reference/index.md) for the canonical field list.
