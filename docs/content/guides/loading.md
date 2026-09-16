# Loading state

Pass `loading` while you're fetching a series and the chart draws a single line
across the plot, undulating slowly and breathing in and out of view.

```tsx
const { candles, isLoading } = useCandles(symbol);

<VroomChart candles={candles} loading={isLoading} />;
```

That's the whole API. When the data lands, the line **becomes** the chart in two
steps: first it reshapes into the series' silhouette, bending to pass through
the vertical centre of every candle about to be drawn; then it fades out while
those candles grow outward from it and their colour fades up. The chart resolves
into place rather than cutting from one scene to another.

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
the candles in the second step of the hand-off.

This is deliberate rather than incidental. The curve is generated and has no
relationship to the asset, so anything that would let a user read a number off
it — a price label, a crosshair readout — would be inventing data. That is also
why the line is drawn dim and never at the contrast of a real series.

## Styling

`theme.skeleton` sets the line's colour, and defaults to inheriting `grid` —
the gridline tone. The gridlines are already how the chart draws structure
rather than data, which is what the line is, so matching them keeps it from
being read as a series.

The line breathes between roughly 60% and 100% of whatever colour it ends up
with. That's a pulse rather than a dimmer, so a recessive colour stays legible;
any alpha you pass multiplies into it, making an opaque colour the usual choice.

To make the line read as the chart's own series warming up instead, hand it the
line colour:

```tsx
<VroomChart candles={candles} loading={isLoading} theme={{ skeleton: '#8957e5' }} />
```

See [Theming](./theming.md) for the rest of the palette.

## Timing and motion

The line fades in over ~300ms, so a request that resolves from cache won't flash
a placeholder on its way past.

The hand-off is driven by the same `transitionMs` / `transitionEasing` props as
the interval switch, so it matches the rest of the chart's motion. Its two steps
split that budget evenly — with the default `transitionMs={300}` the line
reshapes over 150ms and the candles emerge over the next 150ms — so the whole
hand-off costs what any other transition costs. `transitionMs={0}` snaps it.

Under an OS reduced-motion preference the line still draws, but held still: no
drift, no breathing, and no hand-off — the data just appears. On the web this is
detected automatically; on React Native, pass it through `reduceMotion`.

## Why it looks the way it does

The curve is four sine waves summed, at frequencies that aren't multiples of one
another and each drifting at its own rate. A single sine reads as a test
pattern — the repeat is too easy to find — while the sum keeps rearranging
itself.

Its frequency content is tuned to sit next to candlesticks rather than to look
decorative: roughly the texture of real price action at minute resolution, and
drawn through the same monotone spline the line chart uses. That is what makes
the first step of the hand-off read as the same curve coming into focus instead
of one shape being replaced by another.
