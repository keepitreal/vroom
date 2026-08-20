# `timeframeWindow()`

```ts
function timeframeWindow(
  oldWindow,
  oldStepMs,
  oldLastMs,
  newStepMs,
  newLastMs,
): VisibleRange;
```

Source: [react-native/src/dataTransitions.ts:136](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/dataTransitions.ts#L136)

The visible window to apply after a timeframe switch so each candle keeps
the exact pixel width it had before: the visible slot count is preserved and
the right edge re-anchors on the newest candle (any future-gap overshoot is
carried over in slots, clamped to the core's 3/4-window cap). The new start
may precede the first candle — that gap is intentional, width wins.

## Parameters

| Parameter   | Type                                              |
| ----------- | ------------------------------------------------- |
| `oldWindow` | [`VisibleRange`](../type-aliases/VisibleRange.md) |
| `oldStepMs` | `number`                                          |
| `oldLastMs` | `number`                                          |
| `newStepMs` | `number`                                          |
| `newLastMs` | `number`                                          |

## Returns

[`VisibleRange`](../type-aliases/VisibleRange.md)
