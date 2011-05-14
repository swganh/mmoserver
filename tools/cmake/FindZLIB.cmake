find_path(ZLIB_INCLUDE_DIR zlib.h
    HINTS
        $ENV{ZLIB_ROOT}
    PATH_SUFFIXES include
    PATHS
        ${ZLIB_ROOT}
        ${ZLIB_INCLUDEDIR}
)

find_library(ZLIB_LIBRARY_DEBUG
    NAMES z zlib zlibd
    PATH_SUFFIXES lib Debug lib/Debug
    HINTS
        $ENV{ZLIB_ROOT}
        ${ZLIB_ROOT}
        ${ZLIB_LIBRARYDIR}
)

find_library(ZLIB_LIBRARY_RELEASE
    NAMES z zlib
    PATH_SUFFIXES lib Release lib/Release
    HINTS
        $ENV{ZLIB_ROOT}
        ${ZLIB_ROOT}
        ${ZLIB_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLib DEFAULT_MSG ZLIB_LIBRARY_DEBUG ZLIB_LIBRARY_RELEASE ZLIB_INCLUDE_DIR)

mark_as_advanced(ZLIB_LIBRARY_DEBUG ZLIB_LIBRARY_RELEASE ZLIB_INCLUDE_DIR)
