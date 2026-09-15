# Loading state

Pass `loading` while you're fetching a series and the chart draws a single grey
line across the plot, drifting in a slow sine wave.

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

This is deliberate rather than incidental. The line is a sine wave with no
relationship to the asset, so anything that would let a user read a number off
it — a price label, a crosshair readout — would be inventing data.

## Styling

`theme.skeleton` sets the line's colour. It defaults to a neutral grey that sits
between `grid` and `axisText` in value. Any alpha you pass is honoured, then
scaled by the fade-in and the fade-out, so an opaque colour is the usual choice.

```tsx
<VroomChart candles={candles} loading={isLoading} theme={{ skeleton: '#3d444d' }} />
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
drift, and no hand-off — the data just appears. On the web this is detected
automatically; on React Native, pass it through `reduceMotion`.
