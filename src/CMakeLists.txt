cmake_minimum_required(VERSION 2.8.2)

project(mmoserver CXX)

configure_file (
  "${PROJECT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
  )
  
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
    
    SET(_WIN32_WINNT 0x0501 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0501 for Windows XP minimum APIs")
    SET(WINVER 0x0501 CACHE INTERNAL "Setting WINVER to 0x0501 for Windows XP minimum APIs")
	
	# any custom flags that need set for VS compilers only should be set here
	if(MSVC AND NOT CMAKE_GENERATOR MATCHES "Visual Studio 7")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251")
        message(STATUS "- MSVC: Disabled generic compiletime warnings")
    endif()
    
    ADD_DEFINITIONS (/D _WIN32_WINNT=${_WIN32_WINNT})
    MESSAGE(STATUS "- MSVC: Set minimum Windows API version")
    
    ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS /D _SCL_SECURE_NO_WARNINGS /D WIN32_LEAN_AND_MEAN)
    MESSAGE(STATUS "- MSVC: Disabled NON-SECURE warnings")
        
    ADD_DEFINITIONS (/D __TBB_NO_IMPLICIT_LINKAGE=1)
    MESSAGE(STATUS "- MSVC: Disable automatic linking for TBB library")
endif()

option(USE_SYSTEM_LIBRARIES
    "Uses system libraries instead of downloading or building them" FALSE)

# Use the static/multithreaded libraries.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

if(NOT USE_SYSTEM_LIBRARIES)
    set(BOOST_ROOT ${VENDOR_PREFIX}/boost)
    set(GLM_ROOT ${VENDOR_PREFIX}/glm)
    set(GLOG_ROOT ${VENDOR_PREFIX}/glog)
    set(GTEST_ROOT ${VENDOR_PREFIX}/gtest)
    set(GMOCK_ROOT ${VENDOR_PREFIX}/gmock)
    set(LUA_ROOT ${VENDOR_PREFIX}/lua)
    set(ENV{MYSQL_DIR} ${VENDOR_PREFIX}/mysql-connector-c)
    set(MysqlConnectorCpp_ROOT ${VENDOR_PREFIX}/mysql-connector-cpp)
    set(NOISE_ROOT ${VENDOR_PREFIX}/noise)
    set(TBB_INSTALL_DIR ${VENDOR_PREFIX}/tbb)
    set(TOLUAPP_ROOT ${VENDOR_PREFIX}/tolua++)
    set(ZLIB_ROOT ${VENDOR_PREFIX}/zlib)
endif()

find_package(Boost 1.44.0 COMPONENTS date_time regex thread system program_options REQUIRED)
find_package(GLM REQUIRED)
find_package(GLog REQUIRED)
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(Lua REQUIRED)
find_package(MySQL REQUIRED)
find_package(MysqlConnectorCpp REQUIRED)
find_package(Noise REQUIRED)
find_package(TBB REQUIRED)
find_package(ToLuapp REQUIRED)
find_package(ZLIB REQUIRED)

if(WIN32)    
    string(REPLACE "/" "\\" WIN_PROJECT_BINARY_DIR "${PROJECT_BINARY_DIR}")
    
    ## create a target that gathers up the necessary dll's from the above
    ## dependencies and places them in the runtime directory for this project
    add_custom_target(DEPS)

    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${WIN_PROJECT_BINARY_DIR}\\bin"
        VERBATIM
    )

    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)"
        VERBATIM
    )

    get_filename_component(GLOG_DLL_PATH ${GLOG_LIBRARY_DEBUG} PATH)
    string(REPLACE "/" "\\" GLOG_DLL_PATH "${GLOG_DLL_PATH}\\..")
    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND xcopy "${GLOG_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /I /Y /s
        VERBATIM
    )
    
    get_filename_component(MYSQL_CONNECTOR_C_DLL_PATH ${MYSQL_LIB} PATH)
    string(REPLACE "/" "\\" MYSQL_CONNECTOR_C_DLL_PATH "${MYSQL_CONNECTOR_C_DLL_PATH}\\..")
    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND xcopy "${MYSQL_CONNECTOR_C_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /I /Y /s
        VERBATIM
    )
    
    
    get_filename_component(MYSQL_CONNECTOR_CPP_DLL_PATH ${MYSQLCONNECTORCPP_LIBRARY_DEBUG} PATH)
    string(REPLACE "/" "\\" MYSQL_CONNECTOR_CPP_DLL_PATH "${MYSQL_CONNECTOR_CPP_DLL_PATH}\\..")
    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND xcopy "${MYSQL_CONNECTOR_CPP_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /I /Y /s
        VERBATIM
    )
    
    get_filename_component(TBB_CPP_DLL_PATH ${TBB_LIBRARY_DEBUG} PATH)
    string(REPLACE "/" "\\" TBB_CPP_DLL_PATH "${TBB_CPP_DLL_PATH}")
    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND xcopy "${TBB_CPP_DLL_PATH}\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /I /Y /s
        VERBATIM
    )
    
    get_filename_component(ZLIB_DLL_PATH ${ZLIB_LIBRARY_DEBUG} PATH)
    string(REPLACE "/" "\\" ZLIB_DLL_PATH "${ZLIB_DLL_PATH}\\..")
    add_custom_command(
        TARGET DEPS
        POST_BUILD
        COMMAND xcopy "${ZLIB_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /I /Y /s
        VERBATIM
    )    
endif()

enable_testing()

# make sure the source directory and the binary directory (where any generated
# files will be found) are automatically searched for includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# libraries
add_subdirectory(anh)

# executables

# legacy projects
option(DISABLE_LEGACY "disables legacy projects" FALSE)
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
