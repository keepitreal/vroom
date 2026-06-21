cmake_minimum_required(VERSION 3.22)
project(vroom_core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# The native stub static library + unit tests are irrelevant to (and would fail
# under) the emscripten/WASM build, which compiles the full renderer against a
# real Skia. Skip them entirely when VROOM_WASM is set.
option(VROOM_WASM "Build the WebAssembly chart module (requires emscripten)" OFF)

if(NOT VROOM_WASM)

# The core builds as a static library; each platform shim links it in.
add_library(vroom_core STATIC
    src/chart.cpp
)

target_include_directories(vroom_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Skia headers come from third_party/skia (fetched via scripts/fetch-skia.sh).
# Optional for now — the stub doesn't use Skia. Once the renderer lands, this
# will be required.
if(EXISTS "${CMAKE_SOURCE_DIR}/../../third_party/skia/include")
    target_include_directories(vroom_core
        PRIVATE
            ${CMAKE_SOURCE_DIR}/../../third_party/skia/include
    )
endif()

# Warnings on; warnings-as-errors off until the impl stabilizes.
if(MSVC)
    target_compile_options(vroom_core PRIVATE /W4)
else()
    target_compile_options(vroom_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Unit tests (doctest). Off by default so the iOS/podspec build path is
# untouched; enable with -DVROOM_BUILD_TESTS=ON. Compiles only the pure,
# Skia-free translation units directly (not the vroom_core stub lib).
option(VROOM_BUILD_TESTS "Build vroom_core unit tests" OFF)
if(VROOM_BUILD_TESTS)
    enable_testing()
    add_executable(vroom_tests
        tests/doctest_main.cpp
        tests/test_viewport.cpp
        tests/test_ticks.cpp
        tests/test_rsi.cpp
        tests/test_macd.cpp
        tests/test_ma.cpp
        tests/test_vwap.cpp
        src/viewport.cpp
        src/ticks.cpp
        src/rsi.cpp
        src/macd.cpp
        src/ma.cpp
        src/vwap.cpp
    )
    target_include_directories(vroom_tests PRIVATE include src tests)
    add_test(NAME vroom_tests COMMAND vroom_tests)
endif()

endif()  # NOT VROOM_WASM

# WebAssembly build (A1c): the whole core + a Skia-for-WASM static lib compiled
# into one emscripten ES module that owns a WebGL Skia surface and paints a
# <canvas> (web/vroom_web.cpp). Off by default so the native/test paths above are
# untouched. Configure with emscripten, e.g.:
#
#   emcmake cmake -S packages/core -B packages/core/build-wasm -DVROOM_WASM=ON \
#       -DSKIA_DIR=/abs/path/to/skia [-DSKIA_LIB=/abs/path/to/libskia.a]
#   cmake --build packages/core/build-wasm
#
# Output: build-wasm/vroom_core.mjs + vroom_core.wasm (consumed by
# @vroom/core-wasm). See packages/core/web/README.md for the toolchain setup.
if(VROOM_WASM)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR
            "VROOM_WASM=ON requires the emscripten toolchain — configure via emcmake.")
    endif()
    if(NOT DEFINED SKIA_DIR)
        message(FATAL_ERROR
            "Set -DSKIA_DIR=<skia source root> (has include/ and the wasm libskia.a).")
    endif()
    if(NOT DEFINED SKIA_LIB)
        set(SKIA_LIB "${SKIA_DIR}/out/wasm/libskia.a")
    endif()

    # All core TUs (the renderer is split across them; see the podspec glob),
    # plus the web wrapper.
    file(GLOB VROOM_CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    add_executable(vroom_web
        ${VROOM_CORE_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/web/vroom_web.cpp
    )
    target_include_directories(vroom_web PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${SKIA_DIR}
    )
    target_link_libraries(vroom_web PRIVATE ${SKIA_LIB})
    target_compile_options(vroom_web PRIVATE -O3)

    set_target_properties(vroom_web PROPERTIES
        OUTPUT_NAME "vroom_core"
        SUFFIX ".mjs")
    # embind needs RTTI; Skia-WASM disables C++ exception *catching*. WebGL2 on.
    target_link_options(vroom_web PRIVATE
        "-O3"
        "-lembind"
        "-sWASM=1"
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sEXPORT_NAME=createVroomCore"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sGL_ENABLE_GET_PROC_ADDRESS=1"
        "-sEXPORTED_RUNTIME_METHODS=['GL']"
    )
endif()
