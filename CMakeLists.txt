CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(mmoserver CXX)

INCLUDE(ExternalProject)

# Variables
SET(MMOSERVER_VERSION_MAJOR 2)
SET(MMOSERVER_VERSION_MINOR 0)
SET(MMOSERVER_VERSION_PATCH 0)
SET(MMOSERVER_VERSION_VERSION "${MMOSERVER_VERSION_MAJOR}.${MMOSERVER_VERSION_MINOR}.${MMOSERVER_VERSION_PATCH}")

# Load up the custom cmake rules
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")

# On gcc platforms the c++0x flag is needed to enable features used by this project.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # First make sure the version of gnu is new enough
    TRY_COMPILE(HAS_MININUM_GNUCXX_VERSION 
        ${CMAKE_CURRENT_BINARY_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/build-aux/require_gcc_46.cc
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
        
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
    ENDIF()
ENDIF()

# ---------------------------------------------------------------------------
# find -G arg for configuring external projects with the same generator
#
IF(CMAKE_EXTRA_GENERATOR)
  SET(_generator "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
ELSE()
  SET(_generator "${CMAKE_GENERATOR}")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost")
    SET(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/glm")
    SET(GLM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/glm")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest")
    SET(GTEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb")
    SET(TBB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb")
ENDIF()

FIND_PACKAGE(Boost 1.44.0 COMPONENTS date_time thread REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(TBB REQUIRED)

# Add boost and tbb to the include and link directories as everything needs it.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${Boost_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} ${TBB_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

ADD_SUBDIRECTORY(src)
