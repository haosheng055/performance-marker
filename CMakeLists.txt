cmake_minimum_required(VERSION 3.2)

project(PerformanceMarker)

option(PERFORMANCE_MARKER_BUILD_TESTS "Build tests for PerformanceMarker" ON)
option(PERFORMANCE_BUILD_SHARED "Build tests for PerformanceMarker" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

file(GLOB PERFORMANCE_MARKER_SRC_FILES include/* src/*)

add_library(PerformanceMarkerApi STATIC ${PERFORMANCE_MARKER_SRC_FILES})

set_target_properties(PerformanceMarkerApi
        PROPERTIES
        CXX_STANDARD_REQUIRED 11
        )

#if (PERFORMANCE_MARKER_BUILD_TESTS)
add_subdirectory(tests)
add_subdirectory(PerformanceMarker)
#endif ()

