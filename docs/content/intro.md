---
id: intro
slug: /
title: Introduction
sidebar_label: Introduction
description: Lightning-fast, Skia-powered candlestick charts for React Native.
---

# Introduction

**vroom** is a mobile-first candlestick charting library. Its rendering core is
written in C++ on top of [Skia](https://skia.org), so charts stay smooth under
pans, pinches, and live updates — the same engine that powers native iOS and
Android, exposed to React Native through a thin JSI bridge.

This site documents the React Native package, **`react-native-vroom-chart`**.

## Features

- 📈 **Candlesticks + volume** rendered entirely in Skia.
- 🤏 **Gestures** — pan to scroll, pinch to zoom, drag the price/time axes to
  rescale, long-press for a snapping crosshair.
- 🎨 **Theming** — every chart color is overridable.
- 🧮 **Indicators** — RSI and MACD panes, SMA/EMA overlay ribbons, and session
  VWAP.
- 🔔 **Host events** — crosshair OHLCV and viewport changes are surfaced to your
  app (drive your own haptics, tooltips, etc.).

## Get started

- [Installation](getting-started/installation.md) — peer dependencies and the
  native build step.
- [Quickstart](getting-started/quickstart.md) — render your first chart.
- [API reference](reference/index.md) — generated from the source types.

## How these docs are maintained

The **API reference** under `reference/` is generated from the package's
TypeScript types with [TypeDoc](https://typedoc.org) (via
`docusaurus-plugin-typedoc`) — it regenerates on every build, so it can't drift
from the source. Everything else (this page, the guides) is hand-written.

The site is built with [Docusaurus](https://docusaurus.io) and deploys as a
static site (e.g. on Vercel). Run it locally from the repo root with
`pnpm docs` (dev server) or `pnpm docs:build` (production build).
