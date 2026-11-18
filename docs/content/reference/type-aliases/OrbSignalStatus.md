# `OrbSignalStatus`

```ts
type OrbSignalStatus =
  | "entered"
  | "awaitingEntry"
  | "sizeTooSmall"
  | "invalidStop"
  | "afterCutoff"
  | "blockedByTrend"
  | "blockedAfterLoss"
  | "maxSignals"
  | "maxTrades";
```

Source: [types/src/index.ts:1167](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1167)

How a confirmed breakout was handled.
