FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
    HINTS
        $ENV{ZLIB_ROOT}
        ${ZLIB_ROOT}
)
MARK_AS_ADVANCED(ZLIB_INCLUDE_DIR)

FIND_LIBRARY(ZLIB_LIBRARY_DEBUG
    NAMES z zlib zlib.lib
    PATH
        $ENV{ZLIB_ROOT}
        ${ZLIB_ROOT}
    HINTS
        $ENV{ZLIB_ROOT}/projects/visualc6/Win32_LIB_Debug
        ${ZLIB_ROOT}/projects/visualc6/Win32_LIB_Debug
)

FIND_LIBRARY(ZLIB_LIBRARY_RELEASE
    NAMES z zlib zlib.lib
    PATH
        $ENV{ZLIB_ROOT}
        ${ZLIB_ROOT}
    HINTS
        $ENV{ZLIB_ROOT}/projects/visualc6/Win32_LIB_Release
        ${ZLIB_ROOT}/projects/visualc6/Win32_LIB_Release
)
        
IF(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_DEBUG AND ZLIB_LIBRARY_RELEASE)
    SET(ZLIB_FOUND TRUE)
ENDIF()

IF(ZLIB_FOUND)
    IF (NOT ZLIB_FIND_QUIETLY)
        MESSAGE(STATUS "Found ZLIB")
    ENDIF()
ELSE()
    IF (ZLIB_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find ZLIB")
    ENDIF()
ENDIF()
