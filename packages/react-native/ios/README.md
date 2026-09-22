# iOS bridge

A single TurboModule, [`VroomChartModule`](VroomChartModule.mm), conforming to
`RCTTurboModuleWithJSIBindings`. `RCTTurboModuleManager` calls
`-installJSIBindingsWithRuntime:callInvoker:` as soon as it instantiates the
module — before the module object reaches JS — and that installs
`global.VroomChartJSI` via the platform-agnostic
[`../cpp/VroomJsiInstaller.cpp`](../cpp/VroomJsiInstaller.cpp) Android also
uses. There is no native view: the chart renders into an `SkPicture` that
`<Canvas>` from `@shopify/react-native-skia` paints.

This replaced a `[RCTBridge currentBridge]` → `RCTCxxBridge.runtime` lookup,
which stopped working in React Native 0.85. That release enables
`RCT_REMOVE_LEGACY_ARCH` by default, under which `RCTBridge` compiles down to
a stub whose `+currentBridge` returns `nil`.

[`../react-native-vroom-chart.podspec`](../react-native-vroom-chart.podspec)
builds `cpp/` + `ios/` plus the core sources mirrored in from
`packages/core/`.
