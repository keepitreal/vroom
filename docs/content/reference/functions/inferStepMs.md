# `inferStepMs()`

```ts
function inferStepMs(candles): number | null;
```

Source: [react-native/src/dataTransitions.ts:42](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/dataTransitions.ts#L42)

The candle period in ms, inferred as the median of the first few intervals
(robust to a single gap). Null when there are fewer than two candles.

## Parameters

| Parameter | Type                                    |
| --------- | --------------------------------------- |
| `candles` | [`Candle`](../type-aliases/Candle.md)[] |

## Returns

`number` \| `null`
