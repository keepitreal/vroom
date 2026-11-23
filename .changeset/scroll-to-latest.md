---
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Add a headless "back to latest" control. `onAwayFromLatestChange` reports when the user has scrolled far enough into history to show your own button (`awayFromLatestBars` sets how far, default 1 bar), and `chartRef.current.scrollToLatest()` glides the view back to the newest candle, keeping the zoom. Pass `{ animated: false }` to jump, or `durationMs` / `easing` to shape the glide.
