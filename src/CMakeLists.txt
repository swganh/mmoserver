cmake_minimum_required(VERSION 2.8.2)

project(mmoserver CXX)

## compiler checks
include(CheckCXXCompilerFlag)


if(CMAKE_COMPILER_IS_GNUCXX)
    CHECK_CXX_COMPILER_FLAG(-Wno-invalid-offsetof HAS_NO_INVALID_OFFSETOF)
    if(HAS_NO_INVALID_OFFSETOF)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
    endif()

    CHECK_CXX_COMPILER_FLAG(-Wno-write-strings HAS_NO_WRITE_STRINGS)
    if(HAS_NO_WRITE_STRINGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
    endif()

    CHECK_CXX_COMPILER_FLAG(-std=c++0x HAS_CPP0X_SUPPORT)
    IF(HAS_CPP0X_SUPPORT)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    ENDIF()
endif()

if(MSVC)
    if(MSVC_VERSION LESS 1600)
    	message(FATAL_ERROR "Visual Studio 2010 or newer is required ")
    endif()
	
	# any custom flags that need set for VS compilers only should be set here
	if(MSVC AND NOT CMAKE_GENERATOR MATCHES "Visual Studio 7")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251")
        message(STATUS "- MSVC: Disabled generic compiletime warnings")
    endif()
    
    ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS /D _SCL_SECURE_NO_WARNINGS)
    MESSAGE(STATUS "- MSVC: Disabled NON-SECURE warnings")
        
    ADD_DEFINITIONS (/D __TBB_NO_IMPLICIT_LINKAGE=1)
    MESSAGE(STATUS "- MSVC: Disable automatic linking for TBB library")
endif()

# Use the static/multithreaded libraries.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)


if(NOT USE_SYSTEM_LIBRARIES)
    set(BOOST_ROOT ${VENDOR_PREFIX}/boost)
    set(GLM_ROOT ${VENDOR_PREFIX}/glm)
    set(GLOG_ROOT ${VENDOR_PREFIX}/glog)
    set(GTEST_ROOT:PATH=${VENDOR_PREFIX}/gtest)
    set(LUA_ROOT ${VENDOR_PREFIX}/lua)
    set(ENV{MYSQL_DIR} ${VENDOR_PREFIX}/mysql-connector-c)
    set(MysqlConnectorCpp_ROOT ${VENDOR_PREFIX}/mysql-connector-cpp)
    set(NOISE_ROOT ${VENDOR_PREFIX}/noise)
    set(TBB_INSTALL_DIR ${VENDOR_PREFIX}/tbb)
    set(TOLUAPP_ROOT ${VENDOR_PREFIX}/tolua++)
    set(ZLIB_ROOT ${VENDOR_PREFIX}/zlib)
endif()

FIND_PACKAGE(Boost 1.44.0 COMPONENTS date_time regex thread system program_options REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(GLog REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(MySQL REQUIRED)
FIND_PACKAGE(MysqlConnectorCpp REQUIRED)
FIND_PACKAGE(Noise REQUIRED)
FIND_PACKAGE(TBB REQUIRED)
FIND_PACKAGE(ToLuapp REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

enable_testing()

# Add boost and tbb to the include and link directories as everything needs it.
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${GLOG_INCLUDE_DIR} ${GLM_INCLUDE_DIR}  ${TBB_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS} ${TBB_LIBRARY_DIRS})

option(DISABLE_LEGACY "disables legacy projects" FALSE)

# libraries

add_subdirectory(anh)

# executables

if (NOT DISABLE_LEGACY)
    ADD_SUBDIRECTORY(Utils)
    ADD_SUBDIRECTORY(Common)
    ADD_SUBDIRECTORY(NetworkManager)
    ADD_SUBDIRECTORY(DatabaseManager)
    ADD_SUBDIRECTORY(MessageLib)
    ADD_SUBDIRECTORY(SwgProtocol)
    ADD_SUBDIRECTORY(ScriptEngine)
    ADD_SUBDIRECTORY(ChatServer)
    ADD_SUBDIRECTORY(ConnectionServer)
    ADD_SUBDIRECTORY(LoginServer)
    ADD_SUBDIRECTORY(PingServer)
    ADD_SUBDIRECTORY(ZoneServer)
endif()
