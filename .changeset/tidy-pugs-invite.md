---
'react-native-vroom-chart': patch
---

**The podspec no longer bakes an absolute path into your Podfile.lock.** It
resolved the Skia source directory and wrote it straight into
`HEADER_SEARCH_PATHS`, so the spec CocoaPods serializes to `Pods/Local
Podspecs/` — and the `SPEC CHECKSUMS` entry hashed from it — carried the
checkout root of whichever machine last ran `pod install`. CI checks out
somewhere else, computes a different checksum, and `pod install --deployment`
refuses to continue:

```
[!] There were changes to the lockfile in deployment mode:
SPEC CHECKSUMS:
  react-native-vroom-chart:
    New Lockfile: d1dab5b91f8d3fc6baf8d06533c5e13a7587dce9
    Old Lockfile: 5666f29f67dbbba991df5fe7fa107c07b39aae5b
```

No amount of relocking fixed it, because the committed checksum could only ever
match one machine.

The search paths are now emitted relative to `$(PODS_TARGET_SRCROOT)`, computed
when the spec is evaluated rather than hardcoded, so they stay correct under
hoisted and non-hoisted `node_modules` layouts alike. If you are carrying a
local `yarn patch` for this, you can drop it.
