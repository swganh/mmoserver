# AddMMOServerLibrary is the standard way to build a library in the MMOServer
# project. It globs the directory's sources, builds them into a static library
# and, if there are *_unittest.cc/.cpp files, a <name>_tests executable that
# is registered with CTest.
#
# AddMMOServerLibrary(<name>
#     [MMOSERVER_DEPS dep1 ...]    # Other MMOServer libraries this one uses
#     [LIBRARIES lib1 ...]         # Third-party targets this one uses
#     [SOURCES src1 ...]           # Disables the source glob and uses this list
#     [TEST_SOURCES test1 ...])    # Unit tests to use when SOURCES is given
#
# Dependencies are linked PUBLIC, so anything that links <name> also links
# what <name> needs, in an order GNU ld accepts.
#
# Example:
#
# AddMMOServerLibrary(ScriptEngine
#     MMOSERVER_DEPS
#         Utils
#         Common
#     LIBRARIES
#         mmo::lua
#         mmo::tolua
# )

function(AddMMOServerLibrary name)
    cmake_parse_arguments(PARSE_ARGV 1 MMOSERVERLIB "" ""
        "MMOSERVER_DEPS;LIBRARIES;SOURCES;TEST_SOURCES")

    if(MMOSERVERLIB_SOURCES)
        set(SOURCES ${MMOSERVERLIB_SOURCES})
        set(TEST_SOURCES ${MMOSERVERLIB_TEST_SOURCES})
        # main.cpp belongs to the executable, not the library.
        list(FILTER SOURCES EXCLUDE REGEX "/main\\.[^/]*$")
    else()
        file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cc *.cpp *.h)
        file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS *_unittest.cc *_unittest.cpp mock_*.h)
        source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
    endif()

    if(TEST_SOURCES)
        list(REMOVE_ITEM SOURCES ${TEST_SOURCES})
    endif()

    add_library(${name} STATIC ${SOURCES})
    # Sources include each other relative to src/, e.g. "Utils/logger.h".
    target_include_directories(${name} PUBLIC "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(${name} PUBLIC
        ${MMOSERVERLIB_MMOSERVER_DEPS}
        ${MMOSERVERLIB_LIBRARIES}
        mmo::boost
        mmo::glm
        TBB::tbb)

    if(TEST_SOURCES)
        add_executable(${name}_tests ${TEST_SOURCES})
        target_link_libraries(${name}_tests PRIVATE ${name} GTest::gtest_main)
        gtest_discover_tests(${name}_tests
            WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            PROPERTIES TIMEOUT 60
            DISCOVERY_TIMEOUT 60)
    endif()
endfunction()
