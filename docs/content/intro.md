---
id: intro
slug: /
title: Introduction
sidebar_label: Introduction
description: Lightning-fast, Skia-powered candlestick charts for React Native and the web.
---

# vroom 🏎️💨

**vroom** is an open source candlestick charting library that runs on iOS and Android via React Native and web via WASM beneath a React wrapper. The mobile and web implementation share a single C++ rendering core
built on [Skia](https://skia.org), so charts stay smooth under pans, pinches, and
live updates.

|               Mobile (iOS)                |                 Web/Desktop                 |
| :---------------------------------------: | :-----------------------------------------: |
| ![vroom chart on mobile](/img/mobile.png) | ![vroom chart on desktop](/img/desktop.png) |

vroom currently supports 5 technical analysis indicators: RSI, VWAP, EMA, SMA, MACD and volume with more to come.

NOTE: native iOS and Android wrappers are in progress. Building the chart core in C++/Skia means it can and will be truly cross-platform.

## Features

### Candles, Volume and Indicators

Candlesticks and volume render by default. Chart colors, candle styling (wick width, rounded bodies and wicks, rounded volume bars), the initial candle width, indicator visibility and parameters are all configurable through the `theme` and prop surface. vroom does not (currently) offer chart chrome/toolbars to customize the chart and its indicators. This is done by the consuming app so you can have full control of the look feel of your product.

  <img src="/img/desktop-2.png" alt="customizable chart" width="803" />
  <figcaption style={{textAlign: 'left'}}><em>A customized, monochromatic vroom chart on web.</em></figcaption>

## Gestures

Navigating the chart feels natural and performs at 60+ FPS across all interactions.

  <img src="/img/gestures.gif" alt="smooth gestures" width="360" />
  <figcaption style={{textAlign: 'left'}}><em>Industry-standard gestures perform at 60+ FPS</em></figcaption>

<br>
The chart crosshair surfaces chart level data so the host app can render OHLCV data and fire haptics.

```tsx
function Chart({ candles }) {
  return (
    <VroomChart
      candles={candles}
      onCrosshair={(e: CrosshairEvent) => {
        // e.reason → 'hide' || 'show'
        // e.timeMs → // crosshair datetime
        // e.candle → { open, high, low, close, volume } for your OHLCV readout
      }}
    />
  );
}
```

## Drawing tools

Vroom supports trendlines, rectangles and freehand pencil drawing, with full undo/redo (⌘Z / ⇧⌘Z, one step per committed drawing). Drawings are persisted via an injected local storage adapter and are indexed by asset/market so the drawings are maintained across refreshes.

  <img src="/img/drawing.gif" alt="smooth gestures" width="360" />
  <figcaption style={{textAlign: 'left'}}><em>Trendline, rect, and pencil supported</em></figcaption>

## Get started

- [Installation](getting-started/installation.mdx) — packages and peer
  dependencies for each platform.
- [Quickstart](getting-started/quickstart.mdx) — render your first chart.
- [API reference](reference/index.md)
