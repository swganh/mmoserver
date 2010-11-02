# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/glue_files/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/glue_files/*.h)                

INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR} ${TOLUAPP_INCLUDE_DIR} ${NOISE_INCLUDE_DIR} ${GLOG_INCLUDE_DIR})

# Create the Utils library
ADD_LIBRARY(ScriptEngine STATIC ${SOURCES} ${HEADERS})       
ADD_DEPENDENCIES(ScriptEngine
    Utils
    Common
)