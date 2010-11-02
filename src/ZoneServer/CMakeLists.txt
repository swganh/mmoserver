# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES} ${NOISE_INCLUDE_DIR} ${SpatialIndex_INCLUDE_DIR} ${TOLUAPP_INCLUDE_DIR})

# Create the Utils library
ADD_EXECUTABLE(ZoneServer ${SOURCES} ${HEADERS})         
TARGET_LINK_LIBRARIES(ZoneServer
    NetworkManager
    DatabaseManager
    MessageLib
    SwgProtocol
    ScriptEngine
    Common
    Utils
    ${MYSQL_LIB}
    debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
    debug ${Boost_REGEX_LIBRARY_DEBUG}
    debug ${Boost_SYSTEM_LIBRARY_DEBUG}
    debug ${Boost_THREAD_LIBRARY_DEBUG}
    debug ${GLOG_LIBRARY_DEBUG}
    debug ${LUA_LIBRARY_DEBUG}
    debug ${MysqlConnectorCpp_LIBRARY_DEBUG}
    debug ${NOISE_LIBRARY_DEBUG}
    debug ${SpatialIndex_LIBRARY_DEBUG}
    debug ${TBB_LIBRARY_DEBUG}
    debug ${TBB_MALLOC_LIBRARY_DEBUG}
    debug ${TOLUAPP_LIBRARY_DEBUG}
    debug ${ZLIB_LIBRARY_DEBUG}
    optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
    optimized ${Boost_REGEX_LIBRARY_RELEASE}
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
    optimized ${Boost_THREAD_LIBRARY_RELEASE}
    optimized ${GLOG_LIBRARY_RELEASE}
    optimized ${LUA_LIBRARY_RELEASE}
    optimized ${MysqlConnectorCpp_LIBRARY_RELEASE}
    optimized ${NOISE_LIBRARY_RELEASE}
    optimized ${SpatialIndex_LIBRARY_RELEASE}
    optimized ${TBB_LIBRARY}
    optimized ${TBB_MALLOC_LIBRARY}
    optimized ${TOLUAPP_LIBRARY_RELEASE}
    optimized ${ZLIB_LIBRARY_RELEASE}
)

IF(WIN32)
    SET_TARGET_PROPERTIES(ZoneServer PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
	TARGET_LINK_LIBRARIES(ZoneServer "winmm.lib" "ws2_32.lib")
ENDIF(WIN32)
