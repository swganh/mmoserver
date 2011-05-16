find_path(GMOCK_INCLUDE_DIR gmock/gmock.h
    HINTS
        $ENV{GMOCK_ROOT}
    PATH_SUFFIXES include
    PATHS
        ${GMOCK_ROOT}
        ${GMOCK_INCLUDEDIR}
)

find_library(GMOCK_LIBRARY_DEBUG
    NAMES gmock
    PATH_SUFFIXES lib lib/Debug Debug
    HINTS
        $ENV{GMOCK_ROOT}
        ${GMOCK_ROOT}
        ${GMOCK_LIBRARYDIR}
)

find_library(GMOCK_LIBRARY_RELEASE
    NAMES gmock
    PATH_SUFFIXES lib lib/Release Release
    HINTS
        $ENV{GMOCK_ROOT}
        ${GMOCK_ROOT}
        ${GMOCK_LIBRARYDIR}
)

set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR})
if(GMOCK_LIBRARY_RELEASE AND GMOCK_LIBRARY_DEBUG)
    set(GMOCK_LIBRARIES optimized ${GMOCK_LIBRARY_RELEASE} debug ${GMOCK_LIBRARY_DEBUG})
else()
    set(GMOCK_LIBRARIES ${GMOCK_LIBRARY_RELEASE})
endif()

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG GMOCK_LIBRARY_DEBUG GMOCK_LIBRARY_RELEASE GMOCK_INCLUDE_DIR)

mark_as_advanced(GMOCK_LIBRARY_DEBUG GMOCK_LIBRARY_RELEASE GMOCK_INCLUDE_DIR)

