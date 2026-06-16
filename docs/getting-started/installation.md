# Installation

```sh
npm install react-native-vroom-chart
```

## Peer dependencies

vroom renders through Skia and handles gestures with Reanimated worklets, so it
relies on three peer dependencies. Install them if your app doesn't already have
them:

```sh
npm install @shopify/react-native-skia react-native-gesture-handler react-native-reanimated
```

| Package | Version |
| --- | --- |
| [`@shopify/react-native-skia`](https://shopify.github.io/react-native-skia/) | `>= 2.0.0` |
| [`react-native-gesture-handler`](https://docs.swmansion.com/react-native-gesture-handler/) | `>= 2.16.0` |
| [`react-native-reanimated`](https://docs.swmansion.com/react-native-reanimated/) | `>= 4.0.0` |

Wrap your app (or at least the screen containing a chart) in a
`GestureHandlerRootView`, and follow the Reanimated Babel-plugin setup if you
haven't already.

## Native build step

`react-native-vroom-chart` ships a native module (the C++ Skia core via JSI), so
it is **not** available in Expo Go. Use a development build / dev client:

- **Expo:** run a prebuild and install the dev client, then rebuild the native
  app whenever the native module changes.
- **Bare React Native (iOS):** run `pod install` and rebuild. In this repo's test
  bench that's wired up as `pnpm rebuild:ios`.

After the native module is installed, JS-only changes hot-reload normally; you
only need to rebuild when native code changes.

> **Android** support is in progress. Today all platforms are consumed through
> this React Native package; standalone Swift/Kotlin packages are planned.

Next: [Quickstart](quickstart.md).
