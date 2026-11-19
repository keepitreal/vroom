# `OrbTargetMode`

```ts
type OrbTargetMode = "rMultiple" | "holdToFlatten" | "rangeMultiple";
```

Source: [types/src/index.ts:970](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L970)

Where the target goes:
'rMultiple' — `targetR` times the entry-to-stop distance
'holdToFlatten' — no target; exit at the stop or `flattenTime`
'rangeMultiple' — `rangeMult` range widths beyond the breakout level
