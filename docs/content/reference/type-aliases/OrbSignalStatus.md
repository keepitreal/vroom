# `OrbSignalStatus`

```ts
type OrbSignalStatus =
  | "entered"
  | "awaitingEntry"
  | "invalidStop"
  | "afterCutoff"
  | "blockedByTrend"
  | "blockedAfterLoss"
  | "maxSignals"
  | "maxTrades";
```

Source: [types/src/index.ts:1166](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1166)

How a confirmed breakout was handled.
