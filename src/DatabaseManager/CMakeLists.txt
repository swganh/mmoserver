include(MMOServerLibrary)

AddMMOServerLibrary(DatabaseManager
    MMOSERVER_DEPS 
        Utils
        Common
    ADDITIONAL_INCLUDE_DIRS
        ${MYSQL_INCLUDE_DIR}
        ${MysqlConnectorCpp_INCLUDES}
    DEBUG_LIBRARIES
        ${MYSQL_LIB}
        ${MysqlConnectorCpp_LIBRARY_DEBUG}
    OPTIMIZED_LIBRARIES
        ${MYSQL_LIB}
        ${MysqlConnectorCpp_LIBRARY_RELEASE}
)
TARGET_LINK_LIBRARIES(DatabaseManager "-lrt -lpthread")