find_path(LUA_INCLUDE_DIR lua.h
    HINTS
        $ENV{LUA_ROOT}
    PATH_SUFFIXES include src lua5.1
    PATHS
        ${LUA_ROOT}
        ${LUA_INCLUDEDIR}
)

find_library(LUA_LIBRARY_DEBUG
    NAMES lua lua5.1d lua5.1
    PATH_SUFFIXES lib lib/Debug Debug
    HINTS
        $ENV{LUA_ROOT}
        ${LUA_ROOT}
        ${GMOCK_LIBRARYDIR}
)

find_library(LUA_LIBRARY_RELEASE
    NAMES lua lua5.1
    PATH_SUFFIXES lib lib/Release Release
    HINTS
        $ENV{LUA_ROOT}
        ${LUA_ROOT}
        ${GMOCK_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lua DEFAULT_MSG LUA_LIBRARY_DEBUG LUA_LIBRARY_RELEASE LUA_INCLUDE_DIR)

mark_as_advanced(LUA_LIBRARY_DEBUG LUA_LIBRARY_RELEASE LUA_INCLUDE_DIR)
