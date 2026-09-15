# Loading state

Pass `loading` while you're fetching a series and the chart draws a **skeleton**:
a travelling wave of grey placeholder bars, with matching volume bars, gridlines
and pill-shaped stand-ins where the axis labels go.

```tsx
const { candles, isLoading } = useCandles(symbol);

<VroomChart candles={candles} loading={isLoading} />;
```

That's the whole API. When the data lands, the placeholder bars **morph into the
real ones** over `transitionMs`, and their grey blends into each bar's own
bull/bear color — so the chart fills in rather than cutting from one scene to
another.

## The contract

The skeleton shows only when `loading` is true **and** `candles` is empty. Both
halves matter:

- Without `loading`, a series that legitimately has no bars would wave a
  placeholder forever.
- Without the emptiness check, a background refresh would blank out a chart the
  user is already reading. Keep passing the data you have and the real chart
  stays up, interactive, while you refetch.

So hand over an empty array while the request is in flight, which is what a
fetch hook will give you anyway:

```tsx
// Loading: candles is [], loading is true  → skeleton
// Loaded:  candles has bars                → chart (morphs out of the skeleton)
// Refresh: candles has bars, loading true   → chart stays up
```

:::note Switching assets
Clear the candles when the symbol changes. If you leave the previous asset's
bars in place the chart has no way to tell them from the new ones, and it will
keep showing them — priced, labelled and pannable — as if they were the series
you asked for.
:::

## What's suppressed

While the skeleton is up, the chart is inert: no gestures (pan, zoom, crosshair,
drawing), no axis text, no current-price badge, and no indicator panes, even for
indicators you have enabled. All of it comes back with the data.

This is deliberate rather than incidental. The placeholder bars are a random
walk with no relationship to the asset, so anything that would let a user read a
number off them — a price label, a crosshair readout — would be inventing data.

## Styling

One color, `theme.skeleton`, sets the placeholder bars and the axis pills. It
defaults to a neutral grey that sits between `grid` and `axisText` in value.
Supply an opaque color: the wave animation owns the alpha channel, so any alpha
you pass is ignored.

```tsx
<VroomChart candles={candles} loading={isLoading} theme={{ skeleton: '#3d444d' }} />
```

See [Theming](./theming.md) for the rest of the palette.

## Timing and motion

The skeleton fades in over ~300ms, so a request that resolves from cache won't
flash a placeholder on its way past. The hand-off morph is driven by the same
`transitionMs` / `transitionEasing` props as the interval switch, so it matches
the rest of the chart's motion and `transitionMs={0}` snaps it.

Under an OS reduced-motion preference the skeleton still draws, but held still:
no travelling wave, and no hand-off morph — the data just appears. On the web
this is detected automatically; on React Native, pass it through
`reduceMotion`.
