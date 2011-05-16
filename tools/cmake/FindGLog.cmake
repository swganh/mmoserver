find_path(GLOG_INCLUDE_DIR glog/logging.h
    PATH_SUFFIXES include
    HINTS
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
        ${GLOG_INCLUDEDIR}
)

find_library(GLOG_LIBRARY_DEBUG
    NAMES glog libglog
    PATH_SUFFIXES lib lib/Debug Debug
    HINTS
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
        ${GLOG_LIBRARYDIR}
)

find_library(GLOG_LIBRARY_RELEASE
    NAMES glog libglog
    PATH_SUFFIXES lib lib/Release Release
    HINTS
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
        ${GLOG_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLog DEFAULT_MSG GLOG_LIBRARY_DEBUG GLOG_LIBRARY_RELEASE GLOG_INCLUDE_DIR)

mark_as_advanced(GLOG_LIBRARY_DEBUG GLOG_LIBRARY_RELEASE GLOG_INCLUDE_DIR)
