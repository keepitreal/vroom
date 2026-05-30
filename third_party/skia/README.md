# Skia

Skia headers and prebuilt binaries are fetched into this directory by `scripts/fetch-skia.sh`.
Nothing here is committed except this README and `.gitkeep`.

For the RN distribution we don't actually ship Skia ourselves — we borrow the `SkCanvas` from
`@shopify/react-native-skia` at runtime. We still need Skia headers locally to build the core
and the JSI bridge; the script handles that.

For the future standalone iOS/Android packages, we'll ship our own Skia prebuilts here.
