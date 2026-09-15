# `StreamTransition`

```ts
type StreamTransition = "none" | "transform";
```

Source: [types/src/index.ts:228](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L228)

How a live update to the series being displayed animates. `'none'` (default)
applies it on the next frame with no animation and leaves the viewport where
it is. `'transform'` eases the in-progress bar into its new values and, when
the view is already pinned to the newest bar, slides the series left as each
new bar arrives.
