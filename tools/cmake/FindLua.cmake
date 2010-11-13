FIND_PATH(LUA_INCLUDE_DIR lua.h
    HINTS
        $ENV{LUA_ROOT}
        ${LUA_ROOT}
	    $ENV{LUA_ROOT}/include
	    ${LUA_ROOT}/include
        $ENV{LUA_ROOT}/src
        ${LUA_ROOT}/src
)
MARK_AS_ADVANCED(LUA_INCLUDE_DIR)

FIND_LIBRARY(LUA_LIBRARY_DEBUG
    NAMES lua5.1d lua5.1d.lib lua
    PATH
        $ENV{LUA_ROOT}
        ${LUA_ROOT}
    HINTS
        $ENV{LUA_ROOT}/lib
        ${LUA_ROOT}/lib
)

FIND_LIBRARY(LUA_LIBRARY_RELEASE
    NAMES lua lua5.1 lua5.1.lib
    PATH
        $ENV{LUA_ROOT}
        ${LUA_ROOT}
    HINTS
        $ENV{LUA_ROOT}/lib
        ${LUA_ROOT}/lib
)
        
IF(LUA_INCLUDE_DIR AND LUA_LIBRARY_DEBUG AND LUA_LIBRARY_RELEASE)
    SET(LUA_FOUND TRUE)
ENDIF()

IF(LUA_FOUND)
    IF (NOT LUA_FIND_QUIETLY)
        MESSAGE(STATUS "Found LUA")
    ENDIF()
ELSE()
    IF (LUA_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find LUA")
    ENDIF()
ENDIF()
