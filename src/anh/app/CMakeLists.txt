include(ANHPythonBinding)

AddANHPythonBinding(app
    PREFIX_NAME swgpy
    DEPENDS
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager
        MessageLib


    ADDITIONAL_INCLUDE_DIRS
        ${Boost_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR}
        ${MYSQLCONNECTORCPP_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIR}
        ${TBB_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIR}
    DEBUG_LIBRARIES
        ${MYSQL_LIBRARY_DEBUG}
        ${MysqlConnectorCpp_LIBRARY_DEBUG}
        ${MYSQL_LIB}
        ${PYTHON_LIBRARY}
        ${TBB_LIBRARY_DEBUG}
        ${TBB_MALLOC_LIBRARY_DEBUG}
        ${ZLIB_LIBRARY_DEBUG}
	"${CMAKE_SOURCE_DIR}/build/src/DatabaseManager/Debug/DatabaseManager.lib"
	"${CMAKE_SOURCE_DIR}/deps/src/mysql-connector-cpp/driver/Debug/mysqlcppconn.lib"
    OPTIMIZED_LIBRARIES
        ${MYSQL_LIBRARY_RELEASE}
        ${MYSQLCONNECTORCPP_LIBRARY_RELEASE}
        ${MYSQL_LIB}
        ${PYTHON_LIBRARY}
        ${TBB_LIBRARY}
        ${ZLIB_LIBRARY_RELEASE}
        ${TBB_MALLOC_LIBRARY}


)
