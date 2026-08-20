# `DataTransition`

```ts
type DataTransition = "initial" | "stream" | "timeframe" | "reset";
```

Source: [react-native/src/dataTransitions.ts:18](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/dataTransitions.ts#L18)

How a new `candles` array relates to the one the chart already holds:
`'initial'` is the first data, `'stream'` a live update to the same series,
`'timeframe'` the same asset re-bucketed into a different interval, and
`'reset'` a different series entirely.
