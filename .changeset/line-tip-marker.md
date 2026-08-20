---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

**The line chart's newest end is now marked with a dot**, with an optional pulse
expanding out from behind it. Two new boolean theme fields:

```tsx
theme={{ lineTipDot: true, lineTipPulse: true }}
```

`lineTipDot` defaults to **true**, so line-mode charts pick up the dot without
any change — the one visible difference in this release for existing consumers.
Set it to `false` to opt out. Everything about it is derived rather than
separately configurable: the dot takes `line` for its color and scales its radius
off `lineWidth`, wrapped in a 2px ring of `background` that separates it from the
line's own stroke cap.

`lineTipPulse` defaults to **false**. When on, a ring expands out of the dot and
fades, once every 2.6 seconds, with a rest between beats — modeled on
TradingView's last-price animation. Because the ring never settles, turning it on
keeps the chart repainting continuously; leave it off for charts that should be
able to go idle. An OS reduced-motion preference suppresses the pulse (the dot
stays), the same bargain the candle↔line morph already strikes when it keeps its
crossfade but skips the collapse.

The pulse is driven entirely by the core's frame clock rather than a host
animation loop, so it needs nothing from consumers and stays in step with the
candle↔line and interval morphs — the dot rides the line's end through a
timeframe switch instead of detaching from it.

**The default view now leaves a gap between the newest candle and the price
axis**, on candles and the line alike. Previously the newest bar sat flush
against the axis, which read as cramped and left the pulse ring nowhere to expand
into. Charts that pass an explicit `visibleRange` are unaffected, and a timeframe
switch carries the gap over so it stays put.
