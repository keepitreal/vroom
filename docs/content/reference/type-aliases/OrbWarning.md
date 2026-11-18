# `OrbWarning`

```ts
type OrbWarning = {
  code:
    "chartTimeframeTooHigh" | "signalTimeframeNotMultiple" | "invalidTimezone";
  message: string;
};
```

Source: [types/src/index.ts:1275](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1275)

A setup problem:
'chartTimeframeTooHigh' — the chart timeframe exceeds the signal timeframe or range window
'signalTimeframeNotMultiple' — the signal timeframe isn't a whole number of chart bars
'invalidTimezone' — `session.timezone` couldn't be resolved
Signals are disabled while any warning stands.

## Properties

### code

```ts
code:
  | "chartTimeframeTooHigh"
  | "signalTimeframeNotMultiple"
  | "invalidTimezone";
```

Source: [types/src/index.ts:1276](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1276)

---

### message

```ts
message: string;
```

Source: [types/src/index.ts:1277](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1277)
