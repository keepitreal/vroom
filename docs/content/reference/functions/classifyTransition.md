# `classifyTransition()`

```ts
function classifyTransition(prev, next, seriesKeyChanged): DataTransition;
```

Source: [react-native/src/dataTransitions.ts:78](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/dataTransitions.ts#L78)

Classify a candles-prop change. `prev` is the previously rendered array
(null on first render); `seriesKeyChanged` forces `reset` regardless of the
data (the explicit escape hatch).

Constraint: detection compares two immutable snapshots. An array mutated in
place (same reference) never reaches this code — React props must change
identity to re-render.

## Parameters

| Parameter          | Type                                              |
| ------------------ | ------------------------------------------------- |
| `prev`             | [`Candle`](../type-aliases/Candle.md)[] \| `null` |
| `next`             | [`Candle`](../type-aliases/Candle.md)[]           |
| `seriesKeyChanged` | `boolean`                                         |

## Returns

[`DataTransition`](../type-aliases/DataTransition.md)
