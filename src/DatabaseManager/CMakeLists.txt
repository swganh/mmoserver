# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES} ${GLOG_INCLUDE_DIR})

# Create the Utils library
ADD_LIBRARY(DatabaseManager STATIC ${SOURCES} ${HEADERS})         
ADD_DEPENDENCIES(DatabaseManager
    Utils
    Common
)