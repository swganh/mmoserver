
CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
PROJECT(mmoserver CXX)

# Variables
SET(MMOSERVER_VERSION_MAJOR 3)
SET(MMOSERVER_VERSION_MINOR 0)
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

# determine if we are compiling for a 32bit or 64bit system
include(CheckTypeSize)
CHECK_TYPE_SIZE("void*" ANH_PTR_SIZE BUILTIN_TYPES_ONLY LANGUAGE CXX)
if (ANH_PTR_SIZE EQUAL 8)
  set(ANH_PLATFORM_X64 TRUE)
else ()
  set(ANH_PLATFORM_X64 FALSE)
endif ()

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
	SET(CMAKE_EXE_LINKER_FLAGS "-lm -lrt -lpthread -lboost_filesystem -ldl -lboost_log")

    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)

    # when building our libraries are already where we want them to be
    # so build with that path
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # the RPATH to be used when installing
	SET(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/gtest/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/mysql-connector-cpp/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/noise/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/spatialindex/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/tbb/lib:${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib/lib:$ENV{LD_RUN_PATH}")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    
    # On gcc compilers we use an installed directory for locating things.
    SET(__deps_dir deps)
	
	# Fix x64 issues on Linux
	if(ANH_PLATFORM_X64)
		add_definitions(-fPIC)
	endif()
ELSE()
    # If not gcc and on WIN32 we're using visual studio, perform related checks here.
    IF(WIN32) 
        # If using visual studio's compiler make sure we're using at least version 12
        IF(MSVC_VERSION LESS 1800)
            MESSAGE(FATAL_ERROR "MSVC 12 or higher is required to build this library")
        ENDIF()
        
        SET(_WIN32_WINNT 0x0601 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0601 for Windows 7 minimum APIs")
        SET(WINVER 0x0601 CACHE INTERNAL "Setting WINVER to 0x0601 for Windows 7 minimum APIs")

        # If there hasn't been a tbb platform set (probably not on a win32 platform)
        # then set the TBB_ARCH_PLATFORM environment variable manually.
        SET(ENV{TBB_ARCH_PLATFORM} "ia32/vc12")
        
        ADD_DEFINITIONS (/D _WIN32_WINNT=${_WIN32_WINNT})
        MESSAGE(STATUS "- MSVC: Set minimum Windows API version")
        
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS /D _SCL_SECURE_NO_WARNINGS)
        MESSAGE(STATUS "- MSVC: Disabled NON-SECURE warnings")
        
        IF(MSVC)
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251 /wd4786 /wd4503 /wd4275 /wd4290 /wd4661 /wd4996 /wd4127 /wd4100 /wd4005 /wd4075")
          MESSAGE(STATUS "- MSVC: Disabled generic compiletime warnings")
        ENDIF()
		
		if(MSVC)
			if (CMAKE_CL_64)
				# Visual Studio bails out on debug builds in 64bit mode unless
				# this flag is set...
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
				set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
			endif ()
			if (MSVC_VERSION GREATER 1800 OR MSVC_VERSION EQUAL 1800)
				option(ANH_BUILD_MSVC_MP "Enable build with multiple processes in Visual Studio" TRUE)
			else()
				set(ANH_BUILD_MSVC_MP FALSE CACHE BOOL "Compiler option /MP requires at least Visual Studio 2013 (VS12) or newer" FORCE)
			endif()
			
			if(ANH_BUILD_MSVC_MP)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
			endif ()
		endif()
        
        # On windows we use the src dir for dep lookups
        SET(__deps_dir deps/src)
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
    
    SET(GTEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/gtest")
    
    SET(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/lua")
    
    SET(ENV{MYSQL_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/mysql-connector-c")
    
    SET(MysqlConnectorCpp_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/mysql-connector-cpp")
    
    SET(NOISE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/noise")
    
    SET(TBB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/tbb")
    
    SET(TOLUAPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/tolua++")
    
    SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${__deps_dir}/zlib")
ENDIF()

FIND_PACKAGE(Boost 1.58.0 COMPONENTS date_time regex thread system program_options log REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(MySQL REQUIRED)
FIND_PACKAGE(MysqlConnectorCpp REQUIRED)
FIND_PACKAGE(Noise REQUIRED)
FIND_PACKAGE(TBB REQUIRED)
FIND_PACKAGE(ToLuapp REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

# Add boost and tbb to the include and link directories as everything needs it.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${Boost_INCLUDE_DIRS} ${GLM_INCLUDE_DIR}  ${TBB_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

# Enable testing and add gtest to the include directories.
ENABLE_TESTING()

ADD_SUBDIRECTORY(src)
