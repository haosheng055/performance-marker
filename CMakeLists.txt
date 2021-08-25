cmake_minimum_required(VERSION 3.16)

project(PerformanceMarker)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(PERFORMANCE_MARKER_BUILD_TESTS "Build tests for PerformanceMarker" ON)
option(PERFORMANCE_BUILD_SHARED "Build Shared Library For PerformanceMarker" OFF)

# find_package首先会查找cmake_module_Path目录内的<name>.cmake文件
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


file(GLOB PERFORMANCE_MARKER_SRC_FILES src/* src/log/*)
if (PERFORMANCE_BUILD_SHARED)
    add_library(PerformanceMarkerApi SHARED ${PERFORMANCE_MARKER_SRC_FILES})
ELSE ()
    add_library(PerformanceMarkerApi STATIC ${PERFORMANCE_MARKER_SRC_FILES})
endif ()

target_include_directories(PerformanceMarkerApi
        PUBLIC
        include
        )

set_target_properties(PerformanceMarkerApi
        PROPERTIES
        CXX_STANDARD_REQUIRED 17
        )
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17  -pthread")
endif ()

add_subdirectory(PerformanceMarker)
if (PERFORMANCE_MARKER_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

