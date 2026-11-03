# Loading state

Pass `loading` while you're fetching a series and the chart draws a single line
across the plot, undulating slowly and breathing in and out of view.

```tsx
const { candles, isLoading } = useCandles(symbol);

<VroomChart candles={candles} loading={isLoading} />;
```

That's the whole API. When the data lands, the line **becomes** the chart: it
resolves onto the close of every candle, centre first, while the wave keeps
moving underneath, and the candles grow out of it as their colour fades up. In
line mode the line tints into the series line instead. The chart resolves into
place rather than cutting from one scene to another.

## The contract

The line shows only when `loading` is true **and** `candles` is empty. Both
halves matter:

- Without `loading`, a series that legitimately has no bars would draw a
  placeholder forever.
- Without the emptiness check, a background refresh would blank out a chart the
  user is already reading. Keep passing the data you have and the real chart
  stays up, interactive, while you refetch.

So hand over an empty array while the request is in flight, which is what a
fetch hook will give you anyway:

```tsx
// Loading: candles is [], loading is true  → the line
// Loaded:  candles has bars                → chart (the line becomes it)
// Refresh: candles has bars, loading true   → chart stays up
```

If a request fails or resolves to nothing, keep `loading` false and `candles`
empty: the line is dropped rather than left waving at data that isn't coming.

:::note Switching assets
Clear the candles when the symbol changes. If you leave the previous asset's
bars in place the chart has no way to tell them from the new ones, and it will
keep showing them — priced, labelled and pannable — as if they were the series
you asked for.
:::

## What's suppressed

While the line is up, the chart is inert: no gestures (pan, zoom, crosshair,
drawing), no axis text, no current-price badge, and no indicator panes, even for
indicators you have enabled. All of it comes back with the data, arriving with
the candles during the hand-off.

This is deliberate rather than incidental. The curve is generated and has no
relationship to the asset, so anything that would let a user read a number off
it — a price label, a crosshair readout — would be inventing data. That is also
why the line is drawn dim and never at the contrast of a real series.

## Styling

`theme.skeleton` sets the line's colour, and defaults to inheriting `axisText`.
The line is stroked at `theme.lineWidth`, the weight of the series line that
replaces it.

The line breathes between 14% and 30% of whatever colour it ends up with; any
alpha you pass multiplies into it, making an opaque colour the usual choice.

To make the line read as the chart's own series warming up instead, hand it the
line colour:

```tsx
<VroomChart candles={candles} loading={isLoading} theme={{ skeleton: '#8957e5' }} />
```

See [Theming](./theming.md) for the rest of the palette.

## Timing and motion

A chart that opens loading shows the line straight away, as Liveline does; one
that was already showing something fades it in over about half a second. Data
that lands before the chart has been laid out still hands off from the line if
`loading` was set with no candles; a series that's there from the first render,
as from a cache, just appears.

The hand-off runs on its own clock, Liveline's: a fast-out ease that settles in
about 0.9s, which the line, the candles, the price badge and the axes all read,
staggered the way Liveline staggers them. `transitionEasing` doesn't apply to
it, but `transitionMs={0}` still snaps it.

Under an OS reduced-motion preference the line still draws, but held still: no
drift, no breathing, and no hand-off — the data just appears. The OS preference
is detected automatically; pass `reduceMotion` to force it on, say for low-end
devices.

## Why it looks the way it does

The line is adapted from [Liveline](https://github.com/benjitaylor/liveline)'s
loading state, as a single sine wave drifting leftward: a hint that the chart
is alive, kept small and faint so it doesn't compete with the data about to
replace it. It's drawn at `lineWidth`, through the same monotone spline the
line chart uses, so in line mode the hand-off has no renderer seam to hide.
