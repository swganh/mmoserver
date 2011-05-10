FIND_PATH(GLOG_INCLUDE_DIR glog/logging.h
    PATH
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
    HINTS
        $ENV{GLOG_ROOT}/include
        ${GLOG_ROOT}/include
        $ENV{GLOG_ROOT}/src/windows
        ${GLOG_ROOT}/src/windows
)
MARK_AS_ADVANCED(GLOG_INCLUDE_DIR)

FIND_LIBRARY(GLOG_LIBRARY_DEBUG
    NAMES GLOG glog libglog glog.lib libglog.lib
    PATH
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
    HINTS
	    $ENV{GLOG_ROOT}/lib
	    ${GLOG_ROOT}/lib
        $ENV{GLOG_ROOT}/lib/Debug
        ${GLOG_ROOT}/lib/Debug
)

FIND_LIBRARY(GLOG_LIBRARY_RELEASE
    NAMES GLOG glog libglog glog.lib libglog.lib
    PATH
        $ENV{GLOG_ROOT}
        ${GLOG_ROOT}
    HINTS
		$ENV{GLOG_ROOT}/lib
		${GLOG_ROOT}/lib
        $ENV{GLOG_ROOT}/lib/Release
        ${GLOG_ROOT}/lib/Release
)

IF(GLOG_INCLUDE_DIR AND GLOG_LIBRARY_DEBUG AND GLOG_LIBRARY_RELEASE)
    SET(GLOG_FOUND TRUE)
ENDIF()

IF(GLOG_FOUND)
    IF (NOT GLog_FIND_QUIETLY)
        MESSAGE(STATUS "Found GLog")
    ENDIF()
ELSE()
    IF (GLog_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GLog")
    ENDIF()
ENDIF()
