# `IntervalTransition`

```ts
type IntervalTransition = "transform" | "fade";
```

Source: [types/src/index.ts:219](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L219)

How a same-asset interval switch animates. `'transform'` (default) lerps
each visible column into its counterpart. `'fade'` fades the old scene out
then the new one in — use when the two windows don’t share a 1:1 pairing
(e.g. a fixed lookback).
