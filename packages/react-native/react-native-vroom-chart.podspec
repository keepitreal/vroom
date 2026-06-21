require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

# Resolve the absolute path to the @shopify/react-native-skia source.
# We need it because RN-Skia ships its Skia headers via `#include "include/core/SkPicture.h"`,
# which only resolves when the include path points at the source `cpp/` and `cpp/skia/`
# directories — CocoaPods' flat Headers/Public/ layout loses the `include/...` prefix.
skia_pkg_json = `node --print "require.resolve('@shopify/react-native-skia/package.json')"`.strip
skia_src_dir = File.dirname(skia_pkg_json)
skia_cpp_dir = File.join(skia_src_dir, "cpp")
skia_skia_dir = File.join(skia_src_dir, "cpp", "skia")

Pod::Spec.new do |s|
  s.name         = "react-native-vroom-chart"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = "https://github.com/darionwelch/vroom"
  s.license      = { :type => "MIT" }
  s.authors      = { "vroom" => "noreply@example.com" }
  s.platforms    = { :ios => "14.0" }
  s.source       = { :git => "" }

  s.requires_arc = true

  # CocoaPods source_files globs (a) refuse paths outside the spec's source
  # root and (b) don't follow symlinks. We mirror the core sources into a
  # sibling directory inside this pod.
  #
  # In the monorepo (../core present) we re-mirror on every spec evaluation so
  # edits to packages/core/ propagate on the next `pod install`. In a published
  # install there is no ../core — the cpp/_core_{src,include} copies were already
  # vendored into the tarball at publish time (scripts/vendor-core.mjs), so we
  # leave them in place.
  # (Using `prepare_command` doesn't work because CocoaPods caches it per
  # pod version and doesn't re-run for local pods on subsequent installs.)
  require "fileutils"
  spec_dir = __dir__
  core_root = File.join(spec_dir, "..", "core")
  if File.directory?(core_root)
    ["src", "include"].each do |sub|
      src = File.join(core_root, sub)
      dst = File.join(spec_dir, "cpp", "_core_#{sub}")
      FileUtils.rm_rf(dst)
      FileUtils.cp_r(src, dst)
    end
  end

  s.source_files = [
    "ios/**/*.{h,m,mm}",
    "cpp/**/*.{h,cpp}",
    "cpp/_core_src/**/*.{h,cpp}",
    "cpp/_core_include/**/*.h",
  ]

  s.pod_target_xcconfig = {
    "HEADER_SEARCH_PATHS" => [
      '"$(PODS_TARGET_SRCROOT)/cpp/_core_include"',
      '"$(PODS_TARGET_SRCROOT)/cpp/_core_src"',
      '"$(PODS_TARGET_SRCROOT)/cpp"',
      "\"#{skia_cpp_dir}\"",
      "\"#{skia_skia_dir}\"",
    ].join(" "),
    "CLANG_CXX_LANGUAGE_STANDARD" => "c++17",
    # Match RN-Skia's Skia build flags so our SkCanvas/SkPicture code sees the
    # same SK_METAL / SK_GANESH defines and doesn't mis-link.
    "GCC_PREPROCESSOR_DEFINITIONS" =>
      "$(inherited) SK_METAL=1 SK_GANESH=1 SK_DISABLE_LEGACY_SHAPER_FACTORY=1",
  }

  s.dependency "React-Core"
  s.dependency "react-native-skia"

  # Standard new-arch codegen hookup — generates VroomChartSpec from
  # src/NativeVroomChart.ts.
  if defined?(install_modules_dependencies)
    install_modules_dependencies(s)
  end
end
