# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES} ${GLOG_INCLUDE_DIR})

# Create the Utils library
ADD_EXECUTABLE(ChatServer ${SOURCES} ${HEADERS})         
TARGET_LINK_LIBRARIES(ChatServer
    Utils
    Common
    NetworkManager
    DatabaseManager
    ${MYSQL_LIB}
    debug ${GLOG_LIBRARY_DEBUG}
    debug ${MysqlConnectorCpp_LIBRARY_DEBUG}
    debug ${TBB_LIBRARY_DEBUG}
    debug ${TBB_MALLOC_LIBRARY_DEBUG}
    debug ${ZLIB_LIBRARY_DEBUG}
    optimized ${GLOG_LIBRARY_RELEASE}
    optimized ${MysqlConnectorCpp_LIBRARY_RELEASE}
    optimized ${TBB_LIBRARY}
    optimized ${TBB_MALLOC_LIBRARY}
    optimized ${ZLIB_LIBRARY_RELEASE}
)

IF(WIN32)
    SET_TARGET_PROPERTIES(ChatServer PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
	TARGET_LINK_LIBRARIES(ChatServer "winmm.lib" "ws2_32.lib")
ENDIF(WIN32)