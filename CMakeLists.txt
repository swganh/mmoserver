
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(mmoserver CXX)

# Variables
SET(MMOSERVER_VERSION_MAJOR 0)
SET(MMOSERVER_VERSION_MINOR 2)
SET(MMOSERVER_VERSION_PATCH 0)
SET(MMOSERVER_VERSION_VERSION "${MMOSERVER_VERSION_MAJOR}.${MMOSERVER_VERSION_MINOR}.${MMOSERVER_VERSION_PATCH}")

# Load up the custom cmake rules
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")

INCLUDE(ExternalProject)

# Some automated systems need an xml report which requires a second run of the
# tests. To prevent these unnecessary runnings during normal builds the
# following option is set with a default of false.
OPTION(ENABLE_TEST_REPORT
    "Enables the generation of a test report when running tests" FALSE)

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
        
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ")

    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)

    # when building our libraries are already where we want them to be
    # so build with that path
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # the RPATH to be used when installing
    SET(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/glog/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql-connector-cpp/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/noise/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/spatialindex/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/lib:$ENV{LD_RUN_PATH}")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    
    # On gcc compilers we use an installed directory for locating things.
    SET(__deps_dir deps)
ELSE()
    # If not gcc and on WIN32 we're using visual studio, perform related checks here.
    IF(WIN32) 
        # If using visual studio's compiler make sure we're using at least version 10
        IF(MSVC_VERSION LESS 1600)
            MESSAGE(FATAL_ERROR "MSVC 10 or higher is required to build this library")
        ENDIF()
        
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")

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
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251")
          MESSAGE(STATUS "- MSVC: Disabled generic compiletime warnings")
        ENDIF()
        
        # On windows we use the src dir for dep lookups
        SET(__deps_dir deps/src)
    ENDIF()
ENDIF()

# Use the static/multithreaded libraries.
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
<<<<<<< HEAD

=======
        
>>>>>>> parent of 5bd772a... got rid of google log
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

# if the dependencies directory exists, set up some default helpers for the 
# FIND_PACKAGE function to use to find them.
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps")
    IF(WIN32)
        SET(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/boost")
    ELSE()
        SET(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/boost/include")
        SET(BOOST_LIBRARYDIR "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/boost/lib")
    ENDIF()
    
    SET(GLM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/glm")
    
    SET(GLOG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/glog")
    
    SET(GTEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/gtest")
    
    SET(ENV{MYSQL_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/mysql-connector-c")
    
    SET(MysqlConnectorCpp_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/mysql-connector-cpp")
    
    SET(NOISE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/noise")
    
    SET(TBB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/tbb")
    
    SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/zlib")
ENDIF()

<<<<<<< HEAD
FIND_PACKAGE(Boost 1.55.0 COMPONENTS date_time regex thread system program_options log python3 REQUIRED)
=======
FIND_PACKAGE(Boost 1.44.0 COMPONENTS date_time regex thread system program_options REQUIRED)
>>>>>>> parent of 5bd772a... got rid of google log
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(GLog REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(MySQL REQUIRED)
FIND_PACKAGE(MysqlConnectorCpp REQUIRED)
FIND_PACKAGE(Noise REQUIRED)
FIND_PACKAGE(TBB REQUIRED)
find_package(PythonLibs REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

# Add boost and tbb to the include and link directories as everything needs it.
<<<<<<< HEAD
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${Boost_INCLUDE_DIRS} ${GLM_INCLUDE_DIR}  ${TBB_INCLUDE_DIRS}  ${PYTHON_INCLUDE_DIRS}  ${ZLIB_INCLUDE_DIRS})
=======
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${Boost_INCLUDE_DIRS} ${GLOG_INCLUDE_DIR} ${GLM_INCLUDE_DIR}  ${TBB_INCLUDE_DIRS})
>>>>>>> parent of 5bd772a... got rid of google log
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

# Enable testing and add gtest to the include directories.
ENABLE_TESTING()

ADD_SUBDIRECTORY(src)
