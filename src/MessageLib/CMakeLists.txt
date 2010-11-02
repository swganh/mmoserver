# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

INCLUDE_DIRECTORIES(${NOISE_INCLUDE_DIR} ${GLOG_INCLUDE_DIR} ${SpatialIndex_INCLUDE_DIR})

# Create the Utils library
ADD_LIBRARY(MessageLib STATIC ${SOURCES} ${HEADERS})       
ADD_DEPENDENCIES(MessageLib
    Utils
    Common
    NetworkManager
)