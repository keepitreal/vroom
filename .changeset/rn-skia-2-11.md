---
"react-native-vroom-chart": minor
---

Require `@shopify/react-native-skia` ≥ 2.11 and `react-native-worklets`
≥ 0.7. Construct the system font manager through public Skia APIs instead
of RN-Skia's private HostObject context, and wrap pictures with the
NativeState helpers 2.11 introduced. Android now links `libskia.a` from
`react-native-skia-android`.
