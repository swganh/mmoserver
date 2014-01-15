FIND_PATH(SpatialIndex_INCLUDE_DIR spatialindex/SpatialIndex.h
    PATH
        $ENV{SpatialIndex_ROOT}
        ${SpatialIndex_ROOT}
    HINTS
        $ENV{SpatialIndex_ROOT}/include
        ${SpatialIndex_ROOT}/include
)
MARK_AS_ADVANCED(SpatialIndex_INCLUDE_DIR)

FIND_LIBRARY(SpatialIndex_LIBRARY_DEBUG
    NAMES spatialindex spatialindex.lib spatialindex-vc.lib
    PATH
        $ENV{SpatialIndex_ROOT}
        ${SpatialIndex_ROOT}
    HINTS
	    $ENV{SpatialIndex_ROOT}/lib
	    ${SpatialIndex_ROOT}/lib
        $ENV{SpatialIndex_ROOT}/spatialindex-vc/Debug
        ${SpatialIndex_ROOT}/spatialindex-vc/Debug
)

FIND_LIBRARY(SpatialIndex_LIBRARY_RELEASE
    NAMES spatialindex spatialindex.lib spatialindex-vc.lib
    PATH
        $ENV{SpatialIndex_ROOT}
        ${SpatialIndex_ROOT}
    HINTS
		$ENV{SpatialIndex_ROOT}/lib
        ${SpatialIndex_ROOT}/lib
        $ENV{SpatialIndex_ROOT}/spatialindex-vc/Release
        ${SpatialIndex_ROOT}/spatialindex-vc/Release
)
        
IF(SpatialIndex_INCLUDE_DIR AND SpatialIndex_LIBRARY_DEBUG AND SpatialIndex_LIBRARY_RELEASE)
    SET(SpatialIndex_FOUND TRUE)
ENDIF()

IF(SpatialIndex_FOUND)
    IF (NOT SpatialIndex_FIND_QUIETLY)
        MESSAGE(STATUS "Found SpatialIndex")
    ENDIF()
ELSE()
    IF (SpatialIndex_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find SpatialIndex")
    ENDIF()
ENDIF()