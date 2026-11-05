# Locates the third-party libraries and exposes each one as an imported target.
#
# Two setups are supported without any -D flags:
#   * System packages (Debian/Ubuntu apt, see README), found through the
#     packages' own CMake configs or the standard CMake find modules.
#   * vcpkg manifest mode (vcpkg.json + tools/vcpkg/ports overlays), found
#     through the configs vcpkg installs.
#
# Targets provided for the rest of the build:
#   mmo::boost   mmo::glm   mmo::lua   mmo::mysql   mmo::noise
#   mmo::tolua   TBB::tbb   ZLIB::ZLIB GTest::gtest_main

include_guard(GLOBAL)

# CMake 3.30+ removed the FindBoost module; always use Boost's own config.
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Threads REQUIRED)

# --- Boost ---------------------------------------------------------------------
# date_time, system and the other libraries used here are header-only in the
# supported Boost versions; only these need linking.
find_package(Boost 1.70 CONFIG REQUIRED COMPONENTS log program_options regex thread)

add_library(mmo_boost INTERFACE)
add_library(mmo::boost ALIAS mmo_boost)
target_link_libraries(mmo_boost INTERFACE
    Boost::log Boost::program_options Boost::regex Boost::thread Threads::Threads)
if(TARGET Boost::headers)
    target_link_libraries(mmo_boost INTERFACE Boost::headers)
endif()
target_compile_definitions(mmo_boost INTERFACE BOOST_ALL_NO_LIB)
# Boost.Log changes its ABI namespace when linked as a shared library, and
# not every Boost config sets this define for its consumers.
get_target_property(_mmo_boost_log_type Boost::log TYPE)
if(_mmo_boost_log_type STREQUAL "SHARED_LIBRARY")
    target_compile_definitions(mmo_boost INTERFACE BOOST_LOG_DYN_LINK)
endif()

# --- TBB, zlib, GoogleTest -----------------------------------------------------
find_package(TBB CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GTest REQUIRED)

# --- GLM -----------------------------------------------------------------------
find_package(glm CONFIG REQUIRED)
add_library(mmo_glm INTERFACE)
add_library(mmo::glm ALIAS mmo_glm)
target_link_libraries(mmo_glm INTERFACE glm::glm)
# The code uses gtx extensions, which GLM >= 0.9.9 only enables on request.
target_compile_definitions(mmo_glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

# --- Lua 5.1 -------------------------------------------------------------------
# The script engine uses the Lua 5.1 C API (LUA_GLOBALSINDEX, luaL_register),
# so newer Lua versions do not work.
add_library(mmo_lua INTERFACE)
add_library(mmo::lua ALIAS mmo_lua)
find_package(unofficial-lua51 CONFIG QUIET)
if(unofficial-lua51_FOUND)
    target_link_libraries(mmo_lua INTERFACE unofficial::lua51::lua51)
else()
    find_package(Lua51 REQUIRED)
    target_include_directories(mmo_lua INTERFACE ${LUA_INCLUDE_DIR})
    target_link_libraries(mmo_lua INTERFACE ${LUA_LIBRARIES})
endif()

# --- tolua++ -------------------------------------------------------------------
# Only the library is needed: the generated bindings and the matching
# tolua++.h are committed in src/ScriptEngine/glue_files.
add_library(mmo_tolua INTERFACE)
add_library(mmo::tolua ALIAS mmo_tolua)
find_package(unofficial-toluapp CONFIG QUIET)
if(unofficial-toluapp_FOUND)
    target_link_libraries(mmo_tolua INTERFACE unofficial::toluapp::toluapp)
else()
    # Debian/Ubuntu name it libtolua++5.1.
    find_library(TOLUAPP_LIBRARY NAMES tolua++5.1 tolua++ toluapp)
endif()
if(TOLUAPP_LIBRARY)
    target_link_libraries(mmo_tolua INTERFACE ${TOLUAPP_LIBRARY} mmo::lua)
elseif(NOT unofficial-toluapp_FOUND)
    # Ubuntu 26.04 and later no longer package tolua++, so build the runtime
    # library from the same checksummed source as the vcpkg overlay port.
    message(STATUS "tolua++ not found; building it from source")
    enable_language(C)
    include(FetchContent)
    FetchContent_Declare(toluapp
        URL "https://archive.ubuntu.com/ubuntu/pool/universe/t/tolua++/tolua++_1.0.93+repack.orig.tar.gz"
            "https://deb.debian.org/debian/pool/main/t/tolua++/tolua++_1.0.93+repack.orig.tar.gz"
        URL_HASH SHA256=084b6cda15203ebb085b923b3fa30cdb29a91eb506469b8f4477f6f6eff11f35
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        # The tarball has no CMakeLists.txt: only fetch it, the library is
        # defined below.
        SOURCE_SUBDIR no-cmake-project)
    FetchContent_MakeAvailable(toluapp)

    add_library(mmo_toluapp_bundled STATIC
        ${toluapp_SOURCE_DIR}/src/lib/tolua_event.c
        ${toluapp_SOURCE_DIR}/src/lib/tolua_is.c
        ${toluapp_SOURCE_DIR}/src/lib/tolua_map.c
        ${toluapp_SOURCE_DIR}/src/lib/tolua_push.c
        ${toluapp_SOURCE_DIR}/src/lib/tolua_to.c)
    target_include_directories(mmo_toluapp_bundled PRIVATE ${toluapp_SOURCE_DIR}/include)
    target_link_libraries(mmo_toluapp_bundled PUBLIC mmo::lua)
    target_link_libraries(mmo_tolua INTERFACE mmo_toluapp_bundled)
endif()

# --- libnoise ------------------------------------------------------------------
# The code includes <noise/noise.h>. vcpkg installs the headers that way,
# Debian/Ubuntu install them under include/libnoise instead.
add_library(mmo_noise INTERFACE)
add_library(mmo::noise ALIAS mmo_noise)
find_package(unofficial-noise CONFIG QUIET)
if(TARGET unofficial::noise::noise-static)
    target_link_libraries(mmo_noise INTERFACE unofficial::noise::noise-static)
elseif(TARGET unofficial::noise::noise)
    target_link_libraries(mmo_noise INTERFACE unofficial::noise::noise)
else()
    find_library(NOISE_LIBRARY NAMES noise libnoise)
    find_path(NOISE_INCLUDE_DIR noise/noise.h)
    if(NOT NOISE_INCLUDE_DIR)
        find_path(LIBNOISE_INCLUDE_DIR libnoise/noise.h)
        if(LIBNOISE_INCLUDE_DIR)
            # Forward <noise/X.h> to <libnoise/X.h>. The real headers include
            # their own siblings with relative paths, so only the top level
            # needs forwarders.
            set(NOISE_INCLUDE_DIR "${PROJECT_BINARY_DIR}/compat/noise-include")
            file(GLOB _mmo_noise_headers RELATIVE "${LIBNOISE_INCLUDE_DIR}/libnoise"
                "${LIBNOISE_INCLUDE_DIR}/libnoise/*.h")
            foreach(_header IN LISTS _mmo_noise_headers)
                file(CONFIGURE OUTPUT "${NOISE_INCLUDE_DIR}/noise/${_header}"
                    CONTENT "#include <libnoise/${_header}>\n")
            endforeach()
        endif()
    endif()
    if(NOT NOISE_LIBRARY OR NOT NOISE_INCLUDE_DIR)
        message(FATAL_ERROR "libnoise not found (Debian/Ubuntu: libnoise-dev)")
    endif()
    target_include_directories(mmo_noise INTERFACE ${NOISE_INCLUDE_DIR})
    target_link_libraries(mmo_noise INTERFACE ${NOISE_LIBRARY})
endif()

# --- MySQL Connector/C++ (legacy JDBC-style API) --------------------------------
# The code includes <mysql_driver.h> and <cppconn/...>. Debian/Ubuntu install
# them directly in include/, vcpkg in include/jdbc/ (and its target only
# exports include/).
add_library(mmo_mysql INTERFACE)
add_library(mmo::mysql ALIAS mmo_mysql)
find_package(unofficial-mysql-connector-cpp CONFIG QUIET)
find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_driver.h PATH_SUFFIXES jdbc)
if(TARGET unofficial::mysql-connector-cpp::connector-jdbc)
    set(_mmo_mysql_library unofficial::mysql-connector-cpp::connector-jdbc)
else()
    find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn)
    set(_mmo_mysql_library ${MYSQLCPPCONN_LIBRARY})
endif()
if(NOT MYSQLCPPCONN_INCLUDE_DIR OR NOT _mmo_mysql_library)
    message(FATAL_ERROR "MySQL Connector/C++ (legacy API) not found (Debian/Ubuntu: libmysqlcppconn-dev)")
endif()
target_include_directories(mmo_mysql INTERFACE ${MYSQLCPPCONN_INCLUDE_DIR})
target_link_libraries(mmo_mysql INTERFACE ${_mmo_mysql_library})
