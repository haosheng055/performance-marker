project(performance_marker_test)

set(googleTestDir ./lib/googletest)

#Add the google tests subdirectory
add_subdirectory(${googleTestDir})

#include googletest/include dir
include_directories(${googleTestDir}/googletest/include)

#include the googlemock/include dir
include_directories(${googleTestDir}/googlemock/include)

file(GLOB_RECURSE UNITTEST_FILES src/*.cpp)
file(GLOB_RECURSE UNITTEST_FILES_IN_SRC
        ../include/*
        ../src/*
        )

add_executable(performance_marker_unittests
        ${UNITTEST_FILES}
        ${UNITTEST_FILES_IN_SRC})

#Link with GoogleTest
target_link_libraries(performance_marker_unittests
        gtest gtest_main
        gmock gmock_main
        )

set_target_properties(performance_marker_unittests
        PROPERTIES
        CXX_STANDARD 11
        )