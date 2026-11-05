cmake_minimum_required(VERSION 3.21)
project(lua51 VERSION 5.1.5 LANGUAGES C)

# Core and standard libraries; lua.c and luac.c are the stand-alone programs.
set(LUA_SOURCES
    src/lapi.c src/lcode.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c
    src/lgc.c src/llex.c src/lmem.c src/lobject.c src/lopcodes.c src/lparser.c
    src/lstate.c src/lstring.c src/ltable.c src/ltm.c src/lundump.c src/lvm.c
    src/lzio.c
    src/lauxlib.c src/lbaselib.c src/ldblib.c src/liolib.c src/lmathlib.c
    src/loslib.c src/ltablib.c src/lstrlib.c src/loadlib.c src/linit.c)

add_library(lua51 STATIC ${LUA_SOURCES})
target_include_directories(lua51 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/lua5.1>)

if(UNIX)
    target_compile_definitions(lua51 PRIVATE LUA_USE_POSIX LUA_USE_DLOPEN)
    target_link_libraries(lua51 PUBLIC m ${CMAKE_DL_LIBS})
endif()

include(GNUInstallDirs)

install(TARGETS lua51 EXPORT unofficial-lua51-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Same layout as Debian/Ubuntu: include/lua5.1/lua.h
install(FILES src/lua.h src/luaconf.h src/lualib.h src/lauxlib.h etc/lua.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lua5.1)

install(EXPORT unofficial-lua51-targets
    NAMESPACE unofficial::lua51::
    DESTINATION share/unofficial-lua51)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/unofficial-lua51-config.cmake"
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/unofficial-lua51-targets.cmake\")\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/unofficial-lua51-config.cmake"
    DESTINATION share/unofficial-lua51)
