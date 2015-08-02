# AddMMOServerLibrary is a standardized way to build libraries in the MMOServer
# project. Particularly on windows platforms this manages all the
# machinary to set up a default environment and creating/building/running unit
# unit tests an afterthought for developers.
#
# Function Definition:
#
# AddMMOServerLibrary(library_name
#                     MMOSERVER_DEPS [ARGS] [args1...]           # Dependencies on other MMOServer projects
#                     ADDITIONAL_INCLUDE_DIRS [ARGS] [args1...]  # Additional directories to search for includes
#                     ADDITIONAL_SOURCE_DIRS [ARGS] [args1...]   # Additional directories to search for files to include in the project
#                     DEBUG_LIBRARIES [ARGS] [args1....]         # Additional debug libraries to link the project against
#                     OPTIMIZED_LIBRARIES [ARGS] [args1...])     # Additional optimized libraries to link the project against
#
#
########################
# Simple Example Usage:
########################
#
# include(MMOServerLibrary)
# 
# AddMMOServerLibrary(Common
#     MMOSERVER_DEPS 
#         Utils 
# )
#
#
#########################
# Complex Example Usage:
#########################
# include(MMOServerLibrary)
#
# AddMMOServerLibrary(ScriptEngine
#     MMOSERVER_DEPS 
#         Utils
#         Common
#     SOURCES # disables source lookup and uses this list
#         ${SOURCES}
#     TEST_SOURCES # when source lookups are disabled use these tests
#         ${TEST_SOURCES}
#     ADDITIONAL_SOURCE_DIRS
#         ${CMAKE_CURRENT_SOURCE_DIR}/glue_files
#     ADDITIONAL_INCLUDE_DIRS
#         ${LUA_INCLUDE_DIR} 
#         ${NOISE_INCLUDE_DIR} 
#         ${TOLUAPP_INCLUDE_DIR}
#     DEBUG_LIBRARIES
#         ${LUA_LIBRARY_DEBUG}
#         ${NOISE_LIBRARY_DEBUG}
#         ${TOLUAPP_LIBRARY_DEBUG}
#     OPTIMIZED_LIBRARIES
#         ${LUA_LIBRARY_RELEASE}
#         ${NOISE_LIBRARY_RELEASE}
#         ${TOLUAPP_LIBRARY_RELEASE}
# )
#

INCLUDE(CMakeMacroParseArguments)

FUNCTION(AddMMOServerLibrary name)
    PARSE_ARGUMENTS(MMOSERVERLIB "MMOSERVER_DEPS;SOURCES;TEST_SOURCES;ADDITIONAL_INCLUDE_DIRS;ADDITIONAL_SOURCE_DIRS;DEBUG_LIBRARIES;OPTIMIZED_LIBRARIES" "" ${ARGN})
    
    LIST(LENGTH SOURCES __source_files_list_length)
    LIST(LENGTH MMOSERVERLIB_DEBUG_LIBRARIES _debug_list_length)
    LIST(LENGTH MMOSERVERLIB_OPTIMIZED_LIBRARIES _optimized_list_length)
    LIST(LENGTH MMOSERVERLIB_MMOSERVER_DEPS _project_deps_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS _includes_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_SOURCE_DIRS _sources_list_length)
            
    # Grab all of the source files and all of the unit test files.
    IF(__source_files_list_length EQUAL 0)            
        # load up all of the source and header files for the project
        FILE(GLOB_RECURSE SOURCES *.cc *.cpp *.h)
        FILE(GLOB_RECURSE TEST_SOURCES *_unittest.cc *_unittest.cpp mock_*.h)
            
        FOREACH(__source_file ${SOURCES})
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/)((.*/)*)(.*)" "\\2" __source_dir "${__source_file}")
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/${__source_dir})(.*)" "\\2" __source_filename "${__source_file}")
            
            STRING(REPLACE "/" "\\\\" __source_group "${__source_dir}")
            SOURCE_GROUP("${__source_group}" FILES ${__source_file})
        ENDFOREACH()
    ELSE()
        FOREACH(__source_file ${SOURCES})
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/${__source_dir})(.*)" "\\2" __source_filename "${__source_file}")
        
            STRING(SUBSTRING ${__source_filename} 0 5 __main_check)
            STRING(COMPARE EQUAL "main." "${__main_check}" __is_main)
            IF(__is_main)
                LIST(REMOVE_ITEM SOURCES ${__source_file})
            ENDIF()    
        ENDFOREACH()
    ENDIF()
        
    LIST(LENGTH TEST_SOURCES _tests_list_length)    
    IF(_tests_list_length GREATER 0)
        LIST(REMOVE_ITEM SOURCES ${TEST_SOURCES}) # Remove the unit tests from the sources list. 
    ENDIF()
    
    IF(_includes_list_length GREATER 0)
        INCLUDE_DIRECTORIES(${MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS})
    ENDIF()
    
    # Create the Common library
    ADD_LIBRARY(${name} STATIC ${SOURCES})    
    
    IF(_project_deps_list_length GREATER 0)
        ADD_DEPENDENCIES(${name} ${MMOSERVERLIB_MMOSERVER_DEPS})
    ENDIF()

    IF(_tests_list_length GREATER 0)
        # Create an executable for the test and link it to gtest and anh
        INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIR})
        ADD_EXECUTABLE(${name}_tests ${TEST_SOURCES})
        TARGET_LINK_LIBRARIES(${name}_tests 
            ${name}
            ${MMOSERVERLIB_MMOSERVER_DEPS}
            ${GTEST_BOTH_LIBRARIES}
            debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
            debug ${Boost_REGEX_LIBRARY_DEBUG}
            debug ${Boost_SYSTEM_LIBRARY_DEBUG}
            debug ${Boost_THREAD_LIBRARY_DEBUG}
            debug ${Boost_LOG_LIBRARY_DEBUG}
            debug ${TBB_LIBRARY_DEBUG}
            debug ${TBB_MALLOC_LIBRARY_DEBUG}      
            optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
            optimized ${Boost_REGEX_LIBRARY_RELEASE}
            optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
            optimized ${Boost_THREAD_LIBRARY_RELEASE}
            optimized ${Boost_LOG_LIBRARY_RELEASE}
            optimized ${TBB_LIBRARY}
            optimized ${TBB_MALLOC_LIBRARY})
                
        IF(_project_deps_list_length GREATER 0)
            ADD_DEPENDENCIES(${name}_tests ${MMOSERVERLIB_MMOSERVER_DEPS})
        ENDIF()
    
        IF(_debug_list_length GREATER 0)
            TARGET_LINK_LIBRARIES(${name}_tests debug ${MMOSERVERLIB_DEBUG_LIBRARIES})
        ENDIF()
    
        IF(_optimized_list_length GREATER 0)
            TARGET_LINK_LIBRARIES(${name}_tests optimized ${MMOSERVERLIB_OPTIMIZED_LIBRARIES})
        ENDIF()
        
        IF(WIN32)
            # Set the default output directory for binaries for convenience.
            SET_TARGET_PROPERTIES(${name}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
                      
            # Create a custom built user configuration so that the "run in debug mode"
            # works without any issues.
    	    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tools/windows/user_project.vcxproj.in 
    	        ${CMAKE_CURRENT_BINARY_DIR}/${name}_tests.vcxproj.user @ONLY)
    	     
            # After each executable project is built make sure the environment is
    	    # properly set up (scripts, default configs, etc exist).
    	    ADD_CUSTOM_COMMAND(TARGET ${name} POST_BUILD
                COMMAND call \"${PROJECT_SOURCE_DIR}/tools/windows/postbuild.bat\" \"${PROJECT_SOURCE_DIR}\" \"${PROJECT_BINARY_DIR}\" \"\$\(ConfigurationName\)\"
            ) 
               
    	    # After each executable project is built make sure the environment is
    	    # properly set up (scripts, default configs, etc exist).
    	    ADD_CUSTOM_COMMAND(TARGET ${name}_tests POST_BUILD
                COMMAND call \"${PROJECT_BINARY_DIR}/bin/\$\(ConfigurationName\)/${name}_tests\"
            ) 
    	ENDIF()
        
        GTEST_ADD_TESTS(${name}_tests "" ${TEST_SOURCES})
      
        IF(ENABLE_TEST_REPORT)
            ADD_TEST(NAME all_${name}_tests COMMAND ${name}_tests "--gtest_output=xml:${PROJECT_BINARY_DIR}/$<CONFIGURATION>/")
        ENDIF()
    ENDIF()
ENDFUNCTION()