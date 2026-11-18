# `OrbDirectionMode`

```ts
type OrbDirectionMode = "both" | "withRangeCandle" | "longOnly" | "shortOnly";
```

Source: [types/src/index.ts:951](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L951)

Which breakouts may trade:
'both' — longs and shorts
'withRangeCandle' — treat the whole range window as one candle; longs only
if it closed above its midpoint, shorts only if below,
nothing if exactly on it
'longOnly' / 'shortOnly'
