FIND_PATH(NOISE_INCLUDE_DIR noise/noise.h
    PATH
        $ENV{NOISE_ROOT}
        ${NOISE_ROOT}
    HINTS
	    $ENV{NOISE_ROOT}
	    ${NOISE_ROOT}
        $ENV{NOISE_ROOT}/include
        ${NOISE_ROOT}/include
)
MARK_AS_ADVANCED(NOISE_INCLUDE_DIR)

FIND_LIBRARY(NOISE_LIBRARY_DEBUG
    NAMES noise noise.lib libnoise libnoise.lib
    PATH
        $ENV{NOISE_ROOT}
        ${NOISE_ROOT}
    HINTS
		$ENV{NOISE_ROOT}
		${NOISE_ROOT}
	    $ENV{NOISE_ROOT}/lib
	    ${NOISE_ROOT}/lib
        $ENV{NOISE_ROOT}/win32/Debug
        ${NOISE_ROOT}/win32/Debug
)

FIND_LIBRARY(NOISE_LIBRARY_RELEASE
    NAMES noise noise.lib libnoise libnoise.lib
    PATH
        $ENV{NOISE_ROOT}
        ${NOISE_ROOT}
    HINTS
		$ENV{NOISE_ROOT}
		${NOISE_ROOT}
		$ENV{NOISE_ROOT}/lib
		${NOISE_ROOT}/lib
        $ENV{NOISE_ROOT}/win32/Release
        ${NOISE_ROOT}/win32/Release
)
        
IF(NOISE_INCLUDE_DIR AND NOISE_LIBRARY_DEBUG AND NOISE_LIBRARY_RELEASE)
    SET(NOISE_FOUND TRUE)
ENDIF()

IF(NOISE_FOUND)
    IF (NOT NOISE_FIND_QUIETLY)
        MESSAGE(STATUS "Found NOISE")
    ENDIF()
ELSE()
    IF (NOISE_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find NOISE")
    ENDIF()
ENDIF()