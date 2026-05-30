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
