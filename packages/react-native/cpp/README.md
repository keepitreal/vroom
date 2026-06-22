# JSI bridge (placeholder)

When the JS API is stable enough to wire to native, this directory will hold:

- `VroomChartJSI.h` / `VroomChartJSI.cpp` — JSI HostObject(s) exposing the C
  facade from `@vroomchart/core` to JS.
- Glue for borrowing an `SkCanvas` from `@shopify/react-native-skia` and
  passing it to `vroom_chart_draw`.

Until then the JS layer fakes the rendering with RN-Skia primitives so we
can iterate on the component API without native rebuilds.
