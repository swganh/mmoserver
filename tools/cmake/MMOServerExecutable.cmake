# AddMMOServerExecutable is the standard way to build a server executable in
# the MMOServer project. It globs the directory's sources. If the directory
# has unit tests, everything except main.cpp goes into a lib<name> static
# library so the tests can link it.
#
# AddMMOServerExecutable(<name>
#     [MMOSERVER_DEPS dep1 ...]    # Other MMOServer libraries this one uses
#     [LIBRARIES lib1 ...])        # Third-party targets this one uses
#
# Example:
#
# AddMMOServerExecutable(ZoneServer
#     MMOSERVER_DEPS
#         MessageLib
#         ScriptEngine
#     LIBRARIES
#         mmo::noise
# )

include(MMOServerLibrary)

function(AddMMOServerExecutable name)
    cmake_parse_arguments(PARSE_ARGV 1 MMOSERVERLIB "" "" "MMOSERVER_DEPS;LIBRARIES")

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cc *.cpp *.h)
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS *_unittest.cc *_unittest.cpp mock_*.h)
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

    set(_libraries ${MMOSERVERLIB_MMOSERVER_DEPS} ${MMOSERVERLIB_LIBRARIES}
        mmo::boost mmo::glm mmo::mysql TBB::tbb ZLIB::ZLIB)

    if(TEST_SOURCES)
        # Break the project out into a library to make it testable.
        AddMMOServerLibrary(lib${name}
            MMOSERVER_DEPS ${MMOSERVERLIB_MMOSERVER_DEPS}
            LIBRARIES ${MMOSERVERLIB_LIBRARIES}
            SOURCES ${SOURCES}
            TEST_SOURCES ${TEST_SOURCES})
        list(FILTER SOURCES INCLUDE REGEX "/main\\.[^/]*$")
        set(_libraries lib${name} ${_libraries})
    endif()

    add_executable(${name} ${SOURCES})
    target_include_directories(${name} PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(${name} PRIVATE ${_libraries})

    if(WIN32)
        target_link_libraries(${name} PRIVATE winmm ws2_32)
    endif()

    install(TARGETS ${name} RUNTIME DESTINATION bin)
endfunction()
