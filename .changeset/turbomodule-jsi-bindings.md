---
'react-native-vroom-chart': minor
---

Install JSI bindings through the TurboModule bindings hooks instead of the legacy bridge.

React Native 0.85 enables `RCT_REMOVE_LEGACY_ARCH` by default, which compiles `RCTBridge` down to a stub whose `+currentBridge` returns `nil`. The old iOS `install()` path read the JSI runtime from `[RCTBridge currentBridge]`, so on 0.85+ it returned `false` and the chart threw instead of rendering.

iOS now conforms to `RCTTurboModuleWithJSIBindings` and installs `global.VroomChartJSI` from `-installJSIBindingsWithRuntime:callInvoker:`. Android moves to the equivalent `TurboModuleWithJSIBindings` / `BindingsInstallerHolder` API rather than reading a runtime pointer out of `ReactContext.javaScriptContextHolder`. Bindings are now installed when React Native instantiates the module, before it reaches JS.

The `react-native` peer range is now `>=0.78` (previously unbounded) and requires the New Architecture — the bindings hooks have no legacy-bridge equivalent. The podspec's iOS deployment target moves to 15.1 to match React Native's own floor.
