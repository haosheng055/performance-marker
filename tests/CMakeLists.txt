project(PerformanceMarker_unittests)

find_package(Threads REQUIRED)
find_package(gtest REQUIRED)
find_package(gmock REQUIRED)

file(GLOB_RECURSE UNITTEST_FILES src/*)

add_executable(PerformanceMarker_unittests ${UNITTEST_FILES})
target_link_libraries(PerformanceMarker_unittests
        PRIVATE
        $<TARGET_NAME:PerformanceMarkerApi>
        ${GMOCK_LIBRARIES}
        ${GTEST_LIBRARIES}
        Threads::Threads
        )
target_include_directories(PerformanceMarker_unittests
        PRIVATE
        ${GMOCK_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIRS}
        )
set_target_properties(PerformanceMarker_unittests
        PROPERTIES
        CXX_STANDARD 11
        )