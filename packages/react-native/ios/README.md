# iOS shim (placeholder)

When native is wired up, this directory will hold the Objective-C++ view
and module that host the chart and forward gestures into the C++ core.

The accompanying `react-native-vroom-chart.podspec` at the package root will
build `cpp/` + `ios/` + the linked `@vroomchart/core` static library.
