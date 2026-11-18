# `OrbConfig`

```ts
type OrbConfig = {
  confirmation?: {
    confirmCloses?: 1 | 2;
    directionMode?: OrbDirectionMode;
    maxBearishSignals?: number;
    maxBullishSignals?: number;
    maxTradesPerDay?: number;
    minBreakDistance?: number;
    minBreakUnit?: OrbBreakUnit;
    signalTimeframe?: number;
    stopAfterLoss?: boolean;
  };
  display?: {
    backgroundColor?: VroomColor;
    entryColor?: VroomColor;
    highColor?: VroomColor;
    longColor?: VroomColor;
    lossColor?: VroomColor;
    lowColor?: VroomColor;
    midlineColor?: VroomColor;
    onlyShowToday?: boolean;
    openPriceColor?: VroomColor;
    shortColor?: VroomColor;
    showBackgroundFill?: boolean;
    showMidline?: boolean;
    showOpenPrice?: boolean;
    showSignals?: boolean;
    showStatsPanel?: boolean;
    showTradeLevels?: boolean;
    statsPosition?: "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
    stopColor?: VroomColor;
    targetColor?: VroomColor;
    winColor?: VroomColor;
  };
  enabled?: boolean;
  filters?: {
    maxRangeWidthPct?: number | false;
    skipDates?: string[];
    skipWeekdays?: OrbWeekday[];
    trendFilter?: {
      dailyCloses?: {
        close: number;
        timeMs: number;
      }[];
      enabled?: boolean;
      period?: number;
    };
  };
  risk?: {
    allowReversal?: boolean;
    partial?: {
      atR?: number;
      enabled?: boolean;
      moveStopToBreakeven?: boolean;
      pct?: number;
    };
    pointValue?: number;
    rangeMult?: number;
    riskPerTrade?: number;
    stopBufferTicks?: number;
    stopMode?: OrbStopMode;
    stopPoints?: number;
    targetMode?: OrbTargetMode;
    targetR?: number;
    tickSize?: number;
  };
  session?: {
    entryCutoff?: OrbTimeOfDay;
    flattenTime?: OrbTimeOfDay;
    rangeEnd?: OrbTimeOfDay;
    rangePreset?: OrbRangePreset;
    rangeStart?: OrbTimeOfDay;
    timezone?: string;
  };
};
```

Source: [types/src/index.ts:994](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L994)

Settings for the Opening Range Breakout assistant, grouped as in a settings UI.

## Properties

### confirmation?

```ts
optional confirmation?: {
  confirmCloses?: 1 | 2;
  directionMode?: OrbDirectionMode;
  maxBearishSignals?: number;
  maxBullishSignals?: number;
  maxTradesPerDay?: number;
  minBreakDistance?: number;
  minBreakUnit?: OrbBreakUnit;
  signalTimeframe?: number;
  stopAfterLoss?: boolean;
};
```

Source: [types/src/index.ts:1011](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1011)

#### confirmCloses?

```ts
optional confirmCloses?: 1 | 2;
```

Consecutive signal-candle closes beyond the level required. Default 1.

#### directionMode?

```ts
optional directionMode?: OrbDirectionMode;
```

Default 'both'.

#### maxBearishSignals?

```ts
optional maxBearishSignals?: number;
```

Max short signals per day. Default 1.

#### maxBullishSignals?

```ts
optional maxBullishSignals?: number;
```

Max long signals per day. Default 1.

#### maxTradesPerDay?

```ts
optional maxTradesPerDay?: number;
```

Max trades per day. Default 1.

#### minBreakDistance?

```ts
optional minBreakDistance?: number;
```

Extra distance a close must clear the level by. Default 0.

#### minBreakUnit?

```ts
optional minBreakUnit?: OrbBreakUnit;
```

Unit of `minBreakDistance`. Default 'ticks'.

#### signalTimeframe?

```ts
optional signalTimeframe?: number;
```

Timeframe, in minutes, whose candle closes confirm a breakout. Lower chart
timeframes are aggregated up to it. Default 5. A chart timeframe above this
(or above the range window) disables signals with a warning.

#### stopAfterLoss?

```ts
optional stopAfterLoss?: boolean;
```

No further entries that day after a losing stop-out. Default true.

---

### display?

```ts
optional display?: {
  backgroundColor?: VroomColor;
  entryColor?: VroomColor;
  highColor?: VroomColor;
  longColor?: VroomColor;
  lossColor?: VroomColor;
  lowColor?: VroomColor;
  midlineColor?: VroomColor;
  onlyShowToday?: boolean;
  openPriceColor?: VroomColor;
  shortColor?: VroomColor;
  showBackgroundFill?: boolean;
  showMidline?: boolean;
  showOpenPrice?: boolean;
  showSignals?: boolean;
  showStatsPanel?: boolean;
  showTradeLevels?: boolean;
  statsPosition?: "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
  stopColor?: VroomColor;
  targetColor?: VroomColor;
  winColor?: VroomColor;
};
```

Source: [types/src/index.ts:1094](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1094)

#### backgroundColor?

```ts
optional backgroundColor?: VroomColor;
```

The range box's fill, drawn over the candles. Default near-transparent white, `'#14ffffff'` (8% opacity).

#### entryColor?

```ts
optional entryColor?: VroomColor;
```

#### highColor?

```ts
optional highColor?: VroomColor;
```

Range high line, from the window's end to `flattenTime`. Default green.

#### longColor?

```ts
optional longColor?: VroomColor;
```

#### lossColor?

```ts
optional lossColor?: VroomColor;
```

#### lowColor?

```ts
optional lowColor?: VroomColor;
```

Range low line. Default red.

#### midlineColor?

```ts
optional midlineColor?: VroomColor;
```

#### onlyShowToday?

```ts
optional onlyShowToday?: boolean;
```

Draw only the current or most recent session instead of every one loaded. Default false.

#### openPriceColor?

```ts
optional openPriceColor?: VroomColor;
```

#### shortColor?

```ts
optional shortColor?: VroomColor;
```

#### showBackgroundFill?

```ts
optional showBackgroundFill?: boolean;
```

Shade the opening-range candles: a borderless box over them from the
range high to the range low. Default true.

#### showMidline?

```ts
optional showMidline?: boolean;
```

Default false.

#### showOpenPrice?

```ts
optional showOpenPrice?: boolean;
```

A line at the range window's opening price. Default false.

#### showSignals?

```ts
optional showSignals?: boolean;
```

Entry triangles: up below the bar for a long, down above it for a short. Default true.

#### showStatsPanel?

```ts
optional showStatsPanel?: boolean;
```

The stats table over the chart. Default true.

#### showTradeLevels?

```ts
optional showTradeLevels?: boolean;
```

Entry, stop and target lines from the entry bar to the exit. Default true.

#### statsPosition?

```ts
optional statsPosition?: "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
```

Corner the stats table sits in. Default 'topLeft'.

#### stopColor?

```ts
optional stopColor?: VroomColor;
```

#### targetColor?

```ts
optional targetColor?: VroomColor;
```

#### winColor?

```ts
optional winColor?: VroomColor;
```

Exit markers: X at a target (win color) or stop (loss color); a square for a time exit.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:996](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L996)

Run the assistant. Default false.

---

### filters?

```ts
optional filters?: {
  maxRangeWidthPct?: number | false;
  skipDates?: string[];
  skipWeekdays?: OrbWeekday[];
  trendFilter?: {
     dailyCloses?: {
        close: number;
        timeMs: number;
     }[];
     enabled?: boolean;
     period?: number;
  };
};
```

Source: [types/src/index.ts:1035](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1035)

#### maxRangeWidthPct?

```ts
optional maxRangeWidthPct?: number | false;
```

Skip the day if (high − low) / midpoint × 100 exceeds this. Default 0.85;
`false` or 0 disables.

#### skipDates?

```ts
optional skipDates?: string[];
```

Session dates to skip, `'YYYY-MM-DD'` in the session timezone (FOMC, CPI, NFP…).

#### skipWeekdays?

```ts
optional skipWeekdays?: OrbWeekday[];
```

Weekdays to skip. Default none.

#### trendFilter?

```ts
optional trendFilter?: {
  dailyCloses?: {
     close: number;
     timeMs: number;
  }[];
  enabled?: boolean;
  period?: number;
};
```

Longs only above the prior day's daily SMA, shorts only below.

##### trendFilter.dailyCloses?

```ts
optional dailyCloses?: {
  close: number;
  timeMs: number;
}[];
```

Daily closes to compute the SMA from — pass these when the chart's own
intraday history is shorter than `period` days. Without them the chart's
bars are aggregated into daily closes.

##### trendFilter.enabled?

```ts
optional enabled?: boolean;
```

Default false.

##### trendFilter.period?

```ts
optional period?: number;
```

SMA length in days. Default 200.

---

### risk?

```ts
optional risk?: {
  allowReversal?: boolean;
  partial?: {
     atR?: number;
     enabled?: boolean;
     moveStopToBreakeven?: boolean;
     pct?: number;
  };
  pointValue?: number;
  rangeMult?: number;
  riskPerTrade?: number;
  stopBufferTicks?: number;
  stopMode?: OrbStopMode;
  stopPoints?: number;
  targetMode?: OrbTargetMode;
  targetR?: number;
  tickSize?: number;
};
```

Source: [types/src/index.ts:1059](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1059)

#### allowReversal?

```ts
optional allowReversal?: boolean;
```

After a losing stop-out, allow one entry on a confirmed break of the other
side (the "double break"), still subject to `maxTradesPerDay`. Default false.

#### partial?

```ts
optional partial?: {
  atR?: number;
  enabled?: boolean;
  moveStopToBreakeven?: boolean;
  pct?: number;
};
```

Scale out part of the position at an R multiple. Default off.

##### partial.atR?

```ts
optional atR?: number;
```

R multiple to scale out at. Default 1.

##### partial.enabled?

```ts
optional enabled?: boolean;
```

##### partial.moveStopToBreakeven?

```ts
optional moveStopToBreakeven?: boolean;
```

Move the stop to the entry once the partial fills. Default true.

##### partial.pct?

```ts
optional pct?: number;
```

Percent of the position to take, 0–100. Default 50.

#### pointValue?

```ts
optional pointValue?: number;
```

Currency per point per contract. Default 2 (MNQ).

#### rangeMult?

```ts
optional rangeMult?: number;
```

Range widths for 'rangeMultiple'. Default 1.

#### riskPerTrade?

```ts
optional riskPerTrade?: number;
```

Account currency risked per trade, for the suggested size. Default 250.

#### stopBufferTicks?

```ts
optional stopBufferTicks?: number;
```

Extra ticks beyond the stop level. Default 0.

#### stopMode?

```ts
optional stopMode?: OrbStopMode;
```

Default 'oppositeSide'.

#### stopPoints?

```ts
optional stopPoints?: number;
```

Stop distance for 'fixedPoints', in price points. Default 10.

#### targetMode?

```ts
optional targetMode?: OrbTargetMode;
```

Default 'rMultiple'.

#### targetR?

```ts
optional targetR?: number;
```

R multiple for 'rMultiple'. Default 2.

#### tickSize?

```ts
optional tickSize?: number;
```

Minimum price increment, for tick-denominated inputs. Default 0.25 (MNQ).

---

### session?

```ts
optional session?: {
  entryCutoff?: OrbTimeOfDay;
  flattenTime?: OrbTimeOfDay;
  rangeEnd?: OrbTimeOfDay;
  rangePreset?: OrbRangePreset;
  rangeStart?: OrbTimeOfDay;
  timezone?: string;
};
```

Source: [types/src/index.ts:997](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L997)

#### entryCutoff?

```ts
optional entryCutoff?: OrbTimeOfDay;
```

No new entries at or after this time. Default '12:00'.

#### flattenTime?

```ts
optional flattenTime?: OrbTimeOfDay;
```

Any open trade is closed at the close of the bar containing this time. Default '15:55'.

#### rangeEnd?

```ts
optional rangeEnd?: OrbTimeOfDay;
```

Range window end, exclusive. Only read when `rangePreset` is 'custom'. Default '09:45'.

#### rangePreset?

```ts
optional rangePreset?: OrbRangePreset;
```

Range length after `rangeStart`; 'custom' uses `rangeEnd`. Default '15m'.

#### rangeStart?

```ts
optional rangeStart?: OrbTimeOfDay;
```

Range window start. Default '09:30'.

#### timezone?

```ts
optional timezone?: string;
```

IANA timezone the session times are in. Default 'America/New_York'; DST is handled.
