# @vroom/core

Platform-agnostic C++ candlestick chart engine built on Skia. Exposes a thin
`extern "C"` facade (`vroom_chart.h`) so it can be bridged from Swift, JNI, or
JSI without C++ ABI fragility.

## Public API

See [`include/vroom/vroom_chart.h`](include/vroom/vroom_chart.h). The contract
in one paragraph:

- Single-threaded, no internal locking.
- Push render model: core fires `on_redraw_requested`; host draws on next vsync.
- Host owns gesture recognition; calls `pan` / `zoom` / `set_crosshair` directly.
- Data is copied on `set_candles` — caller may free immediately.

## Build

```sh
cmake -S . -B build
cmake --build build
```

CMake is currently optional (the stub has no Skia dependency). Once the
renderer lands, `third_party/skia/include` must be populated via
`scripts/fetch-skia.sh`.

## Layout

- `include/vroom/` — public C headers (the only thing consumers see)
- `src/` — C++ implementation
- `tests/` — gtest unit tests (TODO)
