# Once done these will be defined:
#
#  GMOCK_FOUND
#  GMOCK_INCLUDE_DIRS
#  GMOCK_LIBRARIES
#

include(${CMAKE_CURRENT_LIST_DIR}/DBConfigure.cmake)

find_path(GMOCK_INCLUDE_DIRS
        NAMES
        gmock/gmock.h
        HINTS
        ${DB_INCLUDE_FIND_PATHS}
        )

find_library(GMOCK_LIBRARIES
        NAMES
        gmock
        HINTS
        ${DB_LIB_FIND_PATHS}
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gmock DEFAULT_MSG GMOCK_LIBRARIES GMOCK_INCLUDE_DIRS)
mark_as_advanced(GMOCK_INCLUDE_DIRS GMOCK_LIBRARIES)