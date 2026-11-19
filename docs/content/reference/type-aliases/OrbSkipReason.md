# `OrbSkipReason`

```ts
type OrbSkipReason =
  | "incompleteRange"
  | "maxRangeWidth"
  | "skipDate"
  | "skipWeekday"
  | "trendInsufficientHistory";
```

Source: [types/src/index.ts:986](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L986)

Why a whole session produced no trades:
'incompleteRange' — the data starts after the range window opened,
or no bars fell inside it
'maxRangeWidth' — the range was wider than `maxRangeWidthPct`
'skipDate' / 'skipWeekday' — listed in `skipDates` / `skipWeekdays`
'trendInsufficientHistory' — the trend filter had fewer than `period` prior daily closes
