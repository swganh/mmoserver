# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)          
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)          

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR} ${GLOG_INCLUDE_DIR})

# Create the Common library
ADD_LIBRARY(NetworkManager STATIC ${SOURCES} ${HEADERS})         
ADD_DEPENDENCIES(NetworkManager
    Utils
    Common
)