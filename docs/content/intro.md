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

Candlesticks and volume render by default. Chart colors, indicator visibility and parameters are all configurable. vroom does not (currently) offer chart chrome/toolbars to customize the chart and its indicators. This is done by the consuming app so you can have full control of the look feel of your product.

<figure style={{textAlign: 'center'}}>
  <img src="/img/desktop-2.png" alt="customizable chart" width="803" />
  <figcaption style={{textAlign: 'center'}}><em>A customized, monochromatic vroom chart on web.</em></figcaption>
</figure>

## Gestures

Navigating the chart feels natural and performs at 60+ FPS across all interactions.

<figure style={{textAlign: 'center'}}>
  <img src="/img/gestures.gif" alt="smooth gestures" width="360" />
  <figcaption style={{textAlign: 'center'}}><em>Industry-standard gestures perform at 60+ FPS</em></figcaption>
</figure>

The chart crosshair surfaces chart level data so the host app can render OHLCV data and fire haptics.

```tsx
import { useRef } from "react";
import * as Haptics from "expo-haptics";
import { VroomChart, type CrosshairEvent } from "@vroomchart/react-native";

function Chart({ candles }) {
  // Dedupe on the snapped candle time so haptics fire once per candle, not per pixel.
  const lastTick = useRef<number | null>(null);

  return (
    <VroomChart
      candles={candles}
      onCrosshair={(e: CrosshairEvent) => {
        if (e.reason === "hide") {
          lastTick.current = null;
          return;
        }
        if (e.timeMs !== lastTick.current) {
          lastTick.current = e.timeMs;
          Haptics.selectionAsync(); // tick as the crosshair snaps to a new candle
        }
        // e.candle → { open, high, low, close, volume } for your OHLCV readout
      }}
    />
  );
}
```

## Get started

- [Installation](getting-started/installation.mdx) — packages and peer
  dependencies for each platform.
- [Quickstart](getting-started/quickstart.mdx) — render your first chart.
- [API reference](reference/index.md)
