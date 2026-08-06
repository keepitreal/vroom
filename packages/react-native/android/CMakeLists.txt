# android/CMakeLists.txt
#
# Builds libvroomchart.so from the same platform-agnostic JSI glue and chart
# core the iOS podspec compiles (see ../react-native-vroom-chart.podspec),
# plus this directory's small JNI entry point (src/main/cpp/VroomChartJni.cpp).
#
# RN-Skia's headers (JsiSkHostObjects.h, RNSkPlatformContext.h — needed by
# ../cpp/VroomSkiaContext.cpp) aren't consumed via CMake's prefab mechanism;
# instead we point directly at @shopify/react-native-skia's `cpp/` sources,
# mirroring the podspec's `require.resolve('@shopify/react-native-skia/package.json')`
# trick (VROOM_SKIA_CPP_DIR is computed the same way in build.gradle).
#
# IMPORTANT — SK_GRAPHITE ABI note: RNSkPlatformContext's vtable layout
# differs depending on whether SK_GRAPHITE was defined when RN-Skia's own
# Android build compiled RNSkAndroidPlatformContext (the concrete class whose
# vtable we call into via VroomSkiaContext.cpp's `ctx->createFontMgr()`).
# We must define SK_GRAPHITE identically to however RN-Skia's own CMakeLists
# (android/CMakeLists.txt) decided it, or virtual calls resolve to the wrong
# vtable slot. We replicate their libskia.a symbol probe below rather than
# hard-coding an assumption.
cmake_minimum_required(VERSION 3.22)
project(vroomchart)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ReactAndroid REQUIRED CONFIG)
# Prefab package name matches the Gradle project name AGP autolinking assigns
# RN-Skia (":shopify_react-native-skia"), not the "rnskia" module name inside
# its own `prefab { rnskia { ... } }` block — that block key becomes the
# `shopify_react-native-skia::rnskia` *target* name below, not the package.
find_package(shopify_react-native-skia REQUIRED CONFIG)

if(NOT DEFINED VROOM_SKIA_CPP_DIR)
  message(FATAL_ERROR "VROOM_SKIA_CPP_DIR not set — passed from android/build.gradle.")
endif()

# RN-Skia's own package root is one level up from its cpp/ dir; its prebuilt
# Skia static libs live at <root>/libs/android/<abi>/libskia.a.
get_filename_component(VROOM_SKIA_PKG_DIR "${VROOM_SKIA_CPP_DIR}" DIRECTORY)
set(VROOM_SKIA_LIBS_DIR "${VROOM_SKIA_PKG_DIR}/libs/android/${ANDROID_ABI}")

# Our chart core calls Skia types (SkCanvas, SkPaint, SkFont, ...) and
# RN-Skia's own JSI plumbing (RNJsi::JsiHostObject, JsiSkPicture, ...)
# directly — none of that is header-only, and we need *both* libraries:
#   - librnskia.so (RN-Skia's prefab-published `rnskia` module, consumed above
#     via `implementation project(":shopify_react-native-skia")`) for RNJsi's
#     own symbols (JsiHostObject, JsiSkPicture, ...), which only exist there.
#   - libskia.a (RN-Skia's bundled prebuilt Skia static lib) for direct Skia
#     calls (SkCanvas::drawRect, SkFont, ...): librnskia.so links libskia.a
#     internally but doesn't re-export its symbols, so linking librnskia.so
#     alone leaves every direct Skia call undefined.
# Recompiling RN-Skia's cpp/jsi/*.cpp ourselves instead of linking librnskia.so
# was considered and rejected — it would duplicate RNJsi's symbols (and their
# static/global state) across two separately-loaded .so's, which is fine on
# iOS's single static binary but not here.
add_library(vroom_skia_prebuilt STATIC IMPORTED)
set_property(TARGET vroom_skia_prebuilt PROPERTY
  IMPORTED_LOCATION "${VROOM_SKIA_LIBS_DIR}/libskia.a")

set(SK_GRAPHITE_AVAILABLE OFF)
if(EXISTS "${VROOM_SKIA_LIBS_DIR}/libskia.a")
  execute_process(
    COMMAND nm "${VROOM_SKIA_LIBS_DIR}/libskia.a"
    COMMAND grep "dawn::\\|wgpu\\|_ZN4dawn\\|DawnDevice\\|dawn_native"
    OUTPUT_VARIABLE VROOM_NM_OUTPUT
    ERROR_QUIET
    RESULT_VARIABLE VROOM_NM_RESULT
  )
  if(VROOM_NM_RESULT EQUAL 0 AND NOT "${VROOM_NM_OUTPUT}" STREQUAL "")
    set(SK_GRAPHITE_AVAILABLE ON)
  endif()
endif()

# The chart core (shared across iOS/Android/WASM — see ../../core/CMakeLists.txt).
set(VROOM_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/_core_include")
set(VROOM_CORE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/_core_src")
file(GLOB VROOM_CORE_SOURCES CONFIGURE_DEPENDS "${VROOM_CORE_SRC_DIR}/*.cpp")

add_library(vroomchart SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/VroomJsiInstaller.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/VroomChartHostObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/VroomSkiaContext.cpp"
  ${VROOM_CORE_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/VroomChartJni.cpp"
)

target_include_directories(vroomchart PRIVATE
  "${VROOM_CORE_INCLUDE_DIR}"
  "${VROOM_CORE_SRC_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cpp"
  # RN-Skia headers — mirrors its own android/CMakeLists.txt include list.
  "${VROOM_SKIA_CPP_DIR}"
  "${VROOM_SKIA_CPP_DIR}/api"
  "${VROOM_SKIA_CPP_DIR}/jsi"
  "${VROOM_SKIA_CPP_DIR}/rnskia"
  "${VROOM_SKIA_CPP_DIR}/rnskia/values"
  "${VROOM_SKIA_CPP_DIR}/utils"
  "${VROOM_SKIA_CPP_DIR}/skia"
  "${VROOM_SKIA_CPP_DIR}/dawn/include"
  # RNSkPlatformContext.h pulls in ReactCommon/CallInvoker.h directly, which
  # isn't part of the prefab-published jsi headers — same dirs RN-Skia's own
  # android/CMakeLists.txt adds for the same reason.
  "${REACT_NATIVE_DIR}/ReactCommon"
  "${REACT_NATIVE_DIR}/ReactCommon/callinvoker"
  "${REACT_NATIVE_DIR}/ReactCommon/runtimeexecutor"
)

target_compile_definitions(vroomchart PRIVATE
  SK_BUILD_FOR_ANDROID
  SK_DISABLE_LEGACY_SHAPER_FACTORY
  SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
  ONANDROID
  ON_ANDROID
)
if(SK_GRAPHITE_AVAILABLE)
  target_compile_definitions(vroomchart PRIVATE SK_GRAPHITE)
else()
  target_compile_definitions(vroomchart PRIVATE SK_GL SK_GANESH)
endif()

if(ReactAndroid_VERSION_MINOR GREATER_EQUAL 80)
  include("${REACT_NATIVE_DIR}/ReactCommon/cmake-utils/react-native-flags.cmake")
  target_compile_reactnative_options(vroomchart PRIVATE)
else()
  target_compile_options(vroomchart PRIVATE -fexceptions -frtti -std=c++20 -Wall)
endif()

# ../cpp/_core_include/vroom/vroom_chart.h forward-declares `struct SkCanvas;`
# (to keep the public C facade header Skia-free); Skia itself declares
# `class SkCanvas`. This tag mismatch is harmless but react-native-flags.cmake
# enables -Werror, which would otherwise turn it into a build failure.
target_compile_options(vroomchart PRIVATE -Wno-mismatched-tags)

find_library(VROOM_LOG_LIB log)

target_link_libraries(vroomchart
  ${VROOM_LOG_LIB}
  ReactAndroid::jsi
  shopify_react-native-skia::rnskia
  vroom_skia_prebuilt
)
