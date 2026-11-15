---
"react-native-vroom-chart": patch
---

The chart draws again after React `<Activity>` hides and shows it (and after StrictMode replays its effects). On iOS the UI-thread frame loop stayed stopped once its effect cleanup ran, so every later frame (chart type and timeframe switches, live updates, gestures) was dropped, and the canvas lost its recording when its native view was recreated.
