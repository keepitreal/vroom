---
'react-native-vroom-chart': minor
---

Support React Native Skia v3 (published as `react-native-skia`, Graphite only) alongside v2 (`@shopify/react-native-skia`). The JS shim, podspec and Gradle/CMake build each pick whichever package is installed, and the Android raster path handles v3's `MakeImage` to `MakeRasterImage` rename. Install exactly one of the two Skia packages.
