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
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/require_gcc_46.cc
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
        
        SET(_WIN32_WINNT 0x0501 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0501 for Windows XP minimum APIs")
        SET(WINVER 0x0501 CACHE INTERNAL "Setting WINVER to 0x0501 for Windows XP minimum APIs")

        # If there hasn't been a tbb platform set (probably not on a win32 platform)
        # then set the TBB_ARCH_PLATFORM environment variable manually.
        SET(ENV{TBB_ARCH_PLATFORM} "ia32/vc10")
        
        ADD_DEFINITIONS (/D _WIN32_WINNT=${_WIN32_WINNT})
        MESSAGE(STATUS "- MSVC: Set minimum Windows API version")
        
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS /D _SCL_SECURE_NO_WARNINGS)
        MESSAGE(STATUS "- MSVC: Disabled NON-SECURE warnings")
        
        IF(MSVC AND NOT CMAKE_GENERATOR MATCHES "Visual Studio 7")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
          MESSAGE(STATUS "- MSVC: Disabled generic compiletime warnings")
        ENDIF()  
        
        SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE} CACHE PATH "Executable output path")
    ENDIF()
ENDIF()

# Use the static/multithreaded libraries.
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
        
INCLUDE(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-Wno-invalid-offsetof HAS_NO_INVALID_OFFSETOF)
IF(HAS_NO_INVALID_OFFSETOF)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
ENDIF()

CHECK_CXX_COMPILER_FLAG(-Wno-write-strings HAS_NO_WRITE_STRINGS)
IF(HAS_NO_WRITE_STRINGS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
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
    IF(WIN32)
        SET(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost")
    ELSE()
        SET(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/include")
        SET(BOOST_LIBRARYDIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/lib")
    ENDIF()
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/glm")
    SET(GLM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/glm")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/glog")
    SET(GLOG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/glog")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest")
    SET(GTEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/lua")
    SET(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/lua")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql")
    SET(ENV{MYSQL_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql-connector-cpp")
    SET(MysqlConnectorCpp_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql-connector-cpp")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/noise")
    SET(NOISE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/noise")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/spatialindex")
    SET(SpatialIndex_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/spatialindex")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb")
    SET(TBB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/tolua++")
    SET(TOLUAPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/tolua++")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib")
    SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib")
ENDIF()

FIND_PACKAGE(Boost 1.44.0 COMPONENTS date_time regex thread system REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(GLog REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(MySQL REQUIRED)
FIND_PACKAGE(MysqlConnectorCpp REQUIRED)
FIND_PACKAGE(Noise REQUIRED)
FIND_PACKAGE(SpatialIndex REQUIRED)
FIND_PACKAGE(TBB REQUIRED)
FIND_PACKAGE(ToLuapp REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

# Add boost and tbb to the include and link directories as everything needs it.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${Boost_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} ${TBB_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

# Enable testing and add gtest to the include directories.
ENABLE_TESTING()

ADD_SUBDIRECTORY(src)
