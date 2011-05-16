cmake_minimum_required(VERSION 2.8.2)

project(bootstrap CXX)

# current version
set(mmoserver_VERSION_MAJOR 0)
set(mmoserver_VERSION_MINOR 5)
set(mmoserver_VERSION_PATCH 0)
set(mmoserver_VERSION 
	${mmoserver_VERSION_MAJOR}.${mmoserver_VERSION_MINOR}.${mmoserver_VERSION_PATCH})
	
# set the path for custom cmake scripts
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")

# include commonly used cmake modules
include(ExternalProject)

## Global Configuration Options

# Some automated systems need an xml report which requires a second run of the
# tests. To prevent these unnecessary runnings during normal builds the
# following option is set with a default of false.
OPTION(ENABLE_TEST_REPORT
    "Enables the generation of a test report when running tests" FALSE)
    
option(USE_SYSTEM_LIBRARIES
    "Uses system libraries instead of downloading them" FALSE)

## Vendor libraries
set(vendor_libraries "")

if(MSVC10 AND NOT DEFINED VENDOR_PREFIX AND NOT USE_SYSTEM_LIBRARIES)
	include(FetchPreBuiltDependencies)
	
    # Configure and build the SWGANH project
    ExternalProject_Add(mmoserver
    	DEPENDS ${vendor_libraries}
    	DOWNLOAD_COMMAND ""
    	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/mmoserver
    	CMAKE_ARGS 
    	    -DVENDOR_PREFIX:PATH=${VENDOR_PREFIX}
    	    -Dmmoserver_VERSION_MAJOR=${mmoserver_VERSION_MAJOR}
    	    -Dmmoserver_VERSION_MINOR=${mmoserver_VERSION_MINOR}
    	    -Dmmoserver_VERSION_PATCH=${mmoserver_VERSION_PATCH}
    	    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
    	    -DUSE_SYSTEM_LIBRARIES:BOOL=${USE_SYSTEM_LIBRARIES}
    	INSTALL_COMMAND ""
    )
else()
	add_subdirectory(src)
endif()
