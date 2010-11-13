FIND_PATH(MysqlConnectorCpp_INCLUDE_DIR cppconn/connection.h
    PATH
        $ENV{MysqlConnectorCpp_ROOT}
        ${MysqlConnectorCpp_ROOT}
    HINTS
	    $ENV{MysqlConnectorCpp_ROOT}
	    ${MysqlConnectorCpp_ROOT}
        $ENV{MysqlConnectorCpp_ROOT}/include
        ${MysqlConnectorCpp_ROOT}/include
)
MARK_AS_ADVANCED(MysqlConnectorCpp_INCLUDE_DIR)

FIND_PATH(MysqlConnectorCpp_DRIVER_INCLUDE_DIR mysql_driver.h
    PATH
        $ENV{MysqlConnectorCpp_ROOT}
        ${MysqlConnectorCpp_ROOT}
    HINTS
        $ENV{MysqlConnectorCpp_ROOT}
        ${MysqlConnectorCpp_ROOT}
        $ENV{MysqlConnectorCpp_ROOT}/driver
        ${MysqlConnectorCpp_ROOT}/driver
	    $ENV{MysqlConnectorCpp_ROOT}/include
	    ${MysqlConnectorCpp_ROOT}/include
	    $ENV{MysqlConnectorCpp_ROOT}/include/driver
	    ${MysqlConnectorCpp_ROOT}/include/driver
)
MARK_AS_ADVANCED(MysqlConnectorCpp_DRIVER_INCLUDE_DIR)

FIND_LIBRARY(MysqlConnectorCpp_LIBRARY_DEBUG
    NAMES mysqlcppconn mysqlcppconn.lib mysqlcppconn-static
    PATH
        $ENV{MysqlConnectorCpp_ROOT}
        ${MysqlConnectorCpp_ROOT}
    HINTS
	    $ENV{MysqlConnectorCpp_ROOT}/lib
	    ${MysqlConnectorCpp_ROOT}/lib
        $ENV{MysqlConnectorCpp_ROOT}/driver/Debug
        ${MysqlConnectorCpp_ROOT}/driver/Debug
)

FIND_LIBRARY(MysqlConnectorCpp_LIBRARY_RELEASE
    NAMES mysqlcppconn mysqlcppconn.lib mysqlcppconn-static
    PATH
        $ENV{MysqlConnectorCpp_ROOT}
        ${MysqlConnectorCpp_ROOT}
    HINTS
		$ENV{MysqlConnectorCpp_ROOT}/lib
		${MysqlConnectorCpp_ROOT}/lib
        $ENV{MysqlConnectorCpp_ROOT}/driver/Release
        ${MysqlConnectorCpp_ROOT}/driver/Release
)

IF(MysqlConnectorCpp_INCLUDE_DIR AND MysqlConnectorCpp_LIBRARY_DEBUG AND MysqlConnectorCpp_LIBRARY_RELEASE)
    SET(MysqlConnectorCpp_FOUND TRUE)
ENDIF()

STRING(COMPARE NOTEQUAL MysqlConnectorCpp_INCLUDE_DIR MysqlConnectorCpp_DRIVER_INCLUDE_DIR MysqlConnectorCpp_INCLUDE_DIR_MISMATCH)
IF (MysqlConnectorCpp_INCLUDE_DIR_MISMATCH)
    SET(MysqlConnectorCpp_INCLUDES ${MysqlConnectorCpp_INCLUDE_DIR} ${MysqlConnectorCpp_DRIVER_INCLUDE_DIR})
ELSE()
    SET(MysqlConnectorCpp_INCLUDES ${MysqlConnectorCpp_INCLUDE_DIR})
ENDIF()

IF(MysqlConnectorCpp_FOUND)
    IF (NOT MysqlConnectorCpp_FIND_QUIETLY)
        MESSAGE(STATUS "Found MysqlConnectorCpp")
    ENDIF()
ELSE()
    IF (MysqlConnectorCpp_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find MysqlConnectorCpp")
    ENDIF()
ENDIF()