---
'react-native-vroom-chart': minor
---

Support the `react-native-skia` package name (Skia 2.15+ and 3.x, Graphite on 3.x) alongside `@shopify/react-native-skia` 2.x. The JS shim, podspec and Gradle/CMake build each pick whichever package is installed, Android links the separate Dawn library that Skia 3.x ships, and the raster path handles the `MakeImage` to `MakeRasterImage` rename. Install exactly one of the two Skia packages.
