# Why vroom?

vroom is a specialized candlestick charting library, not a general-purpose charting toolkit. Here's when it's the right choice and how it compares to alternatives.

## When to use vroom

- You need **candlestick charts** with OHLCV data, not bar/line/pie/scatter charts
- You want **60+ FPS gestures** (pan, pinch, zoom) that feel native — not janky canvas redraws
- You're building a **finance or trading app** that needs indicators (RSI, MACD, VWAP, MAs), drawing tools, and a crosshair
- You want **one API across platforms** — the same props and types work on React Native and web
- You need **native performance on mobile** — not a WebView, not a JS-only canvas

## When NOT to use vroom

- You need **general-purpose charts** (bar, line, pie, scatter) — use victory-native, echarts, or recharts
- You only need **simple line charts** with no candlestick or indicator requirements
- You're building a **static data dashboard** where interactivity isn't critical
- You need **server-side rendering** — vroom requires a canvas surface (Skia)

## Comparison

### vroom vs. TradingView lightweight-charts

| | vroom | lightweight-charts |
|---|---|---|
| **Rendering** | Skia (C++), GPU-accelerated | HTML5 Canvas (JS) |
| **Platforms** | React Native (iOS) + Web | Web only |
| **Performance** | 60+ FPS native gestures | Good, but JS-bound |
| **Indicators** | RSI, MACD, VWAP, SMA/EMA, volume | Built into the chart widget |
| **Drawing tools** | Trendlines, boxes, pencil with undo/redo | Lines, shapes (more extensive) |
| **Customization** | Full theme + candle styling knobs | Extensive, but chart chrome is owned by the library |
| **Size** | WASM binary (~2MB) + JS | JS only (~200KB) |

**Choose vroom** if you need native mobile and want full control over the UI chrome. **Choose lightweight-charts** if you're web-only and want TradingView's complete widget (toolbars, menus, built-in drawings).

### vroom vs. victory-native

| | vroom | victory-native |
|---|---|---|
| **Focus** | Candlestick charts only | General-purpose charting |
| **Rendering** | Skia (C++) | react-native-skia (JS-driven) |
| **Gestures** | Built-in pan/pinch/zoom/crosshair | Basic, need manual wiring |
| **Indicators** | Built-in (RSI, MACD, VWAP, MAs) | None (you compute data yourself) |
| **Candlestick** | First-class | Not supported natively |

**Choose vroom** if you specifically need candlestick charts with indicators and native-feel gestures. **Choose victory-native** for general-purpose charts (bar, line, scatter) in React Native.

### vroom vs. react-native-wagmi-charts

| | vroom | wagmi-charts |
|---|---|---|
| **Rendering** | Skia (C++) via JSI | react-native-skia (JS-driven) |
| **Gestures** | 60fps pan/pinch/drag | Basic pan/pinch |
| **Indicators** | 5 built-in indicators | None |
| **Drawing tools** | Trendlines, box, pencil with undo/redo | None |
| **Web support** | Yes (separate `@vroomchart/react` package) | React Native only |

**Choose vroom** for a richer, more performant charting experience with indicators and drawing tools. **Choose wagmi-charts** for a simpler, lighter candlestick component.

## The vroom philosophy

vroom deliberately does **not** provide chart chrome — no built-in toolbars, menus, or preset layouts. It gives you a high-performance rendering surface and lets your app own the surrounding UI. This means:

- Your brand, your design system, your controls
- No fighting the library's defaults
- Complete freedom to build exactly the trading interface you want

The trade-off: you write more UI code. The payoff: your chart looks and feels like *your* app, not someone else's.
