CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(common CXX)

# Load up the custom cmake rules
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../build-aux/cmake")

# On gcc platforms the c++0x flag is needed to enable features used by this project.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # First make sure the version of gnu is new enough
    TRY_COMPILE(HAS_MININUM_GNUCXX_VERSION 
        ${CMAKE_CURRENT_BINARY_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../build-aux/require_gcc_46.cc
        )
        
    IF(NOT HAS_MININUM_GNUCXX_VERSION)
        MESSAGE(FATAL_ERROR "GNU GCC 4.6 or higher is required to build this library")
    ENDIF()
        
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE()
    # If not gcc and on WIN32 we're using visual studio, perform related checks here.
    IF(WIN32) 
        # If using visual studio's compiler make sure we're using at least version 10
        IF(MSVC_VERSION LESS 1600)
            MESSAGE(FATAL_ERROR "MSVC 10 or higher is required to build this library")
        ENDIF()
    
        # If there hasn't been a tbb platform set (probably not on a win32 platform)
        # then set the TBB_ARCH_PLATFORM environment variable manually.
        SET(ENV{TBB_ARCH_PLATFORM} "ia32/vc10")
    
        # Use the static/multithreaded libraries.
        SET(Boost_USE_STATIC_LIBS ON)
        SET(Boost_USE_MULTITHREADED ON)
    ENDIF()
ENDIF()

FIND_PACKAGE(Boost 1.44.0 COMPONENTS date_time thread REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(TBB REQUIRED)

# Add boost and tbb to the include and link directories as everything needs it.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../ ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../utils ${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

# Grab all of the source files and all of the unit test files.
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)          
FILE(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_unittest.cc)
LIST(REMOVE_ITEM SOURCES ${TEST_SOURCES}) # Remove the unit tests from the sources list.        

# Create the libanh shared library
ADD_LIBRARY(common SHARED ${SOURCES})                       
TARGET_LINK_LIBRARIES(common 
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${TBB_LIBRARIES}
  utils
)
