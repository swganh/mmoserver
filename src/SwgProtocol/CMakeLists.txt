# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

# Create the Utils library
ADD_LIBRARY(SwgProtocol STATIC ${SOURCES} ${HEADERS})       
ADD_DEPENDENCIES(SwgProtocol
    Utils
    Common
)