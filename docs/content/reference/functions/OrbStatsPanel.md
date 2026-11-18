# `OrbStatsPanel()`

```ts
function OrbStatsPanel(__namedParameters): Element;
```

Source: react-native/src/OrbStatsPanel.tsx:27

The Opening Range Breakout stats table: trades, win rate, R statistics,
longs vs shorts and days skipped per filter, plus any warning that turned
signals off. `<VroomChart>` renders one when `orb.display.showStatsPanel`
is on; render it yourself from `onOrbResult` to place it elsewhere.

## Parameters

| Parameter           | Type                                                          |
| ------------------- | ------------------------------------------------------------- |
| `__namedParameters` | [`OrbStatsPanelProps`](../type-aliases/OrbStatsPanelProps.md) |

## Returns

`Element`
