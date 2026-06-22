---
id: intro
slug: /
title: Introduction
sidebar_label: Introduction
description: Lightning-fast, Skia-powered candlestick charts for React Native and the web.
---

# Introduction

**vroom** is a candlestick charting library with a single C++ rendering core
built on [Skia](https://skia.org), so charts stay smooth under pans, pinches, and
live updates. That one core powers two front-ends:

- **`react-native-vroom-chart`** — native iOS/Android, exposed to React Native
  through a thin JSI bridge.
- **`@vroomchart/react`** — the web/DOM component, backed by
  **`@vroomchart/core-wasm`** (the same core compiled to WebAssembly, painting a
  `<canvas>`).

Both expose the **same props and events**, so this site documents them together:
platform-specific bits (install, imports, input idioms) are shown in **React
Native / Web** tabs — pick your platform once and it sticks across every page.

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

- [Installation](getting-started/installation.mdx) — packages and peer
  dependencies for each platform.
- [Quickstart](getting-started/quickstart.mdx) — render your first chart.
- [API reference](reference/index.md) — generated from the source types (see
  [Platform differences](reference/platform-differences) for the per-platform props).

## How these docs are maintained

The **API reference** under `reference/` is generated from the package's
TypeScript types with [TypeDoc](https://typedoc.org) (via
`docusaurus-plugin-typedoc`) — it regenerates on every build, so it can't drift
from the source. Everything else (this page, the guides) is hand-written.

The site is built with [Docusaurus](https://docusaurus.io) and deploys as a
static site (e.g. on Vercel). Run it locally from the repo root with
`pnpm docs` (dev server) or `pnpm docs:build` (production build).
