cmake_minimum_required(VERSION 3.22)
project(vroom_core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
