# Once done these will be defined:
#
#  GTEST_FOUND
#  GTEST_INCLUDE_DIRS
#  GTEST_LIBRARIES
#

include(${CMAKE_CURRENT_LIST_DIR}/DBConfigure.cmake)

find_path(GTEST_INCLUDE_DIRS
        NAMES
        gtest/gtest.h
        HINTS
        ${DB_INCLUDE_FIND_PATHS}
        )

find_library(GTEST_LIBRARY
        NAMES
        gtest
        HINTS
        ${DB_LIB_FIND_PATHS}
        )

find_library(GTEST_MAIN_LIBRARIES
        NAMES
        gtest_main
        HINTS
        ${DB_LIB_FIND_PATHS}
        )

if (GTEST_LIBRARY AND GTEST_MAIN_LIBRARIES)
    set(GTEST_LIBRARIES
            ${GTEST_LIBRARY}
            ${GTEST_MAIN_LIBRARIES}
            )
else ()
    set(GTEST_LIBRARIES
            GTEST_LIBRARIES-NOTFOUND
            )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gtest DEFAULT_MSG GTEST_LIBRARIES GTEST_INCLUDE_DIRS)
mark_as_advanced(GTEST_INCLUDE_DIRS GTEST_LIBRARIES)