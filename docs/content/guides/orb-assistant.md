# ORB Assistant

The Opening Range Breakout (ORB) Assistant marks the high and low of a session's
first minutes, waits for a candle to close beyond one of them, then simulates a
trade with a stop, a target and a time exit, and keeps score in R (multiples of
the initial risk). It draws on the chart, reports a stats table, and reports events as they happen.

It works on **both web and React Native**. The strategy itself lives in the
public `@vroomchart/strategies` package as a pure function with no drawing code;
the chart wrappers only read its output and draw it.

## Turning it on

```tsx
import { VroomChart, type OrbEvent } from "@vroomchart/react";

<VroomChart
  candles={candles}
  orb={{ enabled: true }}
  onOrbEvent={(e: OrbEvent) => notify(`${e.type} ${e.direction ?? ""}`)}
/>;
```

With no other settings it trades the New York open: a 15-minute range from 09:30,
confirmed on 5-minute closes, one trade a day, no new entries after 12:00, and
everything flat at 15:55.

It makes no assumption about what you trade: futures, options, spot or anything
else with candles. Trades carry prices, and every result is in R. Position
sizing is left to your app: risk per unit is `|entryPrice − stop|` on each trade.

## What it draws

- **The range.** A shaded box over the range window, plus high and low lines from
  the window's end to the flatten time. An optional midline and an optional line
  at the window's opening price.
- **Signals.** An up triangle under the bar for a long, a down triangle over it
  for a short, labeled with the trade's risk in price (`Short · risk 7.25`, the
  distance from the entry to the stop).
- **Trade levels.** Entry, stop and target lines from the entry bar to the exit.
- **Exits.** An X at a target (win color) or a stop (loss color), a square at a
  time exit, and the realized R beside it.
- **Skipped days.** A label naming the reason: a skip date or weekday, a range
  too wide, an incomplete range.

Every session loaded is drawn. Set `display.onlyShowToday` to draw only the
current or most recent one; stats always cover every session loaded.

## The rules

Everything is deterministic and never looks ahead:

1. **The range** is built from bars whose open time falls inside
   `[rangeStart, rangeEnd)` in `session.timezone` (DST is handled). It becomes
   final at the close of its last bar. A day is skipped as an incomplete range
   when the loaded data starts after its window opened, or when the window
   passed with no bars in it.
2. **A breakout** needs `confirmCloses` consecutive signal-candle closes beyond
   the level, by at least `minBreakDistance`. When the chart timeframe is lower
   than `signalTimeframe`, bars are aggregated up to it. A candle that is still
   forming never confirms. While price is already through, it shows as pending.
3. **The entry** is the open of the bar after the confirming close. The stop is
   the opposite side of the range by default, and the target is 2R.
4. **Each bar**, in order: a gap through the stop or target fills at the open; a
   stop touch exits (the stop wins when both are touched in the same bar); then
   the partial, then the target; finally a flatten at the close of the bar
   containing `flattenTime`.

A chart timeframe above the signal timeframe or the range window disables signals
and shows a warning in the stats table, rather than guessing.


## Events and results

`onOrbEvent` fires once per new event: `rangeFinalized`, `breakoutConfirmed`,
`entry`, `targetHit`, `stopHit`, `timeExit` and `daySkipped`. Every event has a
stable `id`, so re-renders never repeat one. History already on the chart when
it loads, or when the series or config changes, is not replayed.

The chart doesn't notify anyone itself. Pick the event types you care about and
route them wherever you like:

```tsx
<VroomChart
  candles={candles}
  orb={{ enabled: true }}
  onOrbEvent={(e) => {
    if (e.type === "entry" || e.type === "stopHit" || e.type === "targetHit") {
      toast(`${e.type} ${e.direction} @ ${e.price}`);
    }
  }}
/>;
```

`onOrbResult` receives the full `OrbResult` after every run: sessions, trades,
events, aggregate `stats` and any `warnings`. Use it to build your own panel, and
set `display.showStatsPanel: false` to hide the built-in one.

## Running it without a chart

The engine is exported for backtests, servers and tests:

```ts
import { runOrb } from "@vroomchart/strategies";

const result = runOrb(candles, { session: { rangePreset: "30m" } });
console.log(result.stats.netR, result.stats.winRate);
```

`runOrb` treats every bar as closed unless you pass `{ nowMs: Date.now() }`,
which the chart does so the forming bar is never traded. The same bars, config
and `nowMs` always produce the same result.

## Common settings

```ts
orb={{
  enabled: true,
  session: { timezone: "Europe/London", rangeStart: "08:00", rangePreset: "30m" },
  confirmation: { confirmCloses: 2, directionMode: "withRangeCandle" },
  filters: { maxRangeWidthPct: 0.6, skipDates: ["2026-12-09"] },
  risk: { targetMode: "rMultiple", targetR: 1.5, partial: { enabled: true, atR: 1 } },
  display: { showMidline: true, statsPosition: "topRight" },
}}
```

See `OrbConfig` in the API reference for every option and its default.
