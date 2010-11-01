# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)  
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)                

# Create the Utils library
ADD_LIBRARY(Utils STATIC ${SOURCES} ${HEADERS})                       
TARGET_LINK_LIBRARIES(Utils 
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${TBB_LIBRARIES}
)