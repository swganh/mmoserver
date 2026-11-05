cmake_minimum_required(VERSION 3.21)
project(toluapp VERSION 1.0.93 LANGUAGES C)

find_package(unofficial-lua51 CONFIG REQUIRED)

add_library(toluapp STATIC
    src/lib/tolua_event.c src/lib/tolua_is.c src/lib/tolua_map.c
    src/lib/tolua_push.c src/lib/tolua_to.c)
target_include_directories(toluapp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(toluapp PUBLIC unofficial::lua51::lua51)

include(GNUInstallDirs)

install(TARGETS toluapp EXPORT unofficial-toluapp-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES include/tolua++.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT unofficial-toluapp-targets
    NAMESPACE unofficial::toluapp::
    DESTINATION share/unofficial-toluapp)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/unofficial-toluapp-config.cmake" [[
include(CMakeFindDependencyMacro)
find_dependency(unofficial-lua51 CONFIG)
include("${CMAKE_CURRENT_LIST_DIR}/unofficial-toluapp-targets.cmake")
]])
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/unofficial-toluapp-config.cmake"
    DESTINATION share/unofficial-toluapp)
