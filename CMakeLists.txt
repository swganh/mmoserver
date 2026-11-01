cmake_minimum_required(VERSION 3.21)

project(mmoserver
    VERSION 3.0.0
    DESCRIPTION "Star Wars Galaxies pre-CU server emulator"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Debug build for single-config generators when nothing was chosen.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")

# Put every executable in one directory so the servers and the staged data
# (config/, script/, heightmaps/) sit side by side.
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif()

if(MSVC)
    add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00
        _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)

    # Legacy warnings, kept disabled until the warning cleanup in phase 3:
    # /wd4244 - conversion warnings (e.g., double to float, int64 to int32)
    # /wd4251 - class needs to have dll-interface (STL in DLL boundaries)
    # /wd4786 - identifier truncated in debug info (legacy, not relevant in modern MSVC)
    # /wd4503 - decorated name length exceeded (template-heavy code)
    # /wd4275 - non dll-interface base class (DLL export issues)
    # /wd4290 - C++ exception specification ignored (deprecated in C++17)
    # /wd4661 - no suitable definition for explicit template instantiation
    # /wd4996 - deprecated function warnings (e.g., strcpy vs strcpy_s)
    # /wd4127 - conditional expression is constant (common in templates)
    # /wd4100 - unreferenced formal parameter
    # /wd4005 - macro redefinition
    # /wd4075 - initializers put in unrecognized initialization area
    add_compile_options(/wd4244 /wd4251 /wd4786 /wd4503 /wd4275 /wd4290
        /wd4661 /wd4996 /wd4127 /wd4100 /wd4005 /wd4075 /bigobj /MP)
else()
    add_compile_options(-Wno-invalid-offsetof -Wno-write-strings)
endif()

include(MMOServerDependencies)

enable_testing()
include(GoogleTest)

add_subdirectory(src)

# `cmake --install` lays out a runnable server: binaries plus the data they
# load relative to the working directory.
install(DIRECTORY data/config data/script DESTINATION bin)
