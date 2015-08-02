# AddMMOServerExecutable is a standardized way to build binary executables in
# the MMOServer project. Particularly on windows platforms this manages all the
# machinary to set up a default environment to make building and then running
# a simple task.
#
# Function Definition:
#
# AddMMOServerExecutable(executable_name
#                        MMOSERVER_DEPS [ARGS] [args1...]           # Dependencies on other MMOServer projects
#                        ADDITIONAL_INCLUDE_DIRS [ARGS] [args1...]  # Additional directories to search for includes
#                        ADDITIONAL_SOURCE_DIRS [ARGS] [args1...]   # Additional directories to search for files to include in the project
#                        DEBUG_LIBRARIES [ARGS] [args1....]         # Additional debug libraries to link the project against
#                        OPTIMIZED_LIBRARIES [ARGS] [args1...])     # Additional optimized libraries to link the project against
#
#
########################
# Simple Example Usage:
########################
#
# include(MMOServerExecutable)
#
# AddMMOServerExecutable(LoginServer)
#
#
#########################
# Complex Example Usage:
#########################
# include(MMOServerExecutable)
# 
# AddMMOServerExecutable(ZoneServer
#     MMOSERVER_DEPS 
#         MessageLib
#         SwgProtocol
#         ScriptEngine
#     ADDITIONAL_INCLUDE_DIRS
#         ${LUA_INCLUDE_DIR} 
#         ${NOISE_INCLUDE_DIR} 
#         ${SpatialIndex_INCLUDE_DIR} 
#         ${TOLUAPP_INCLUDE_DIR}
#     ADDITIONAL_SOURCE_DIRS
#         ${CMAKE_CURRENT_SOURCE_DIR}/objects
#     DEBUG_LIBRARIES 
#         ${LUA_LIBRARY_DEBUG}
#         ${NOISE_LIBRARY_DEBUG}
#         ${SpatialIndex_LIBRARY_DEBUG}
#         ${TOLUAPP_LIBRARY_DEBUG}
#     OPTIMIZED_LIBRARIES
#         ${LUA_LIBRARY_RELEASE}
#         ${NOISE_LIBRARY_RELEASE}
#         ${SpatialIndex_LIBRARY_RELEASE}
#         ${TOLUAPP_LIBRARY_RELEASE}
# )
#

INCLUDE(CMakeMacroParseArguments)
INCLUDE(MMOServerLibrary)

FUNCTION(AddMMOServerExecutable name)
    PARSE_ARGUMENTS(MMOSERVERLIB "MMOSERVER_DEPS;ADDITIONAL_INCLUDE_DIRS;ADDITIONAL_SOURCE_DIRS;DEBUG_LIBRARIES;OPTIMIZED_LIBRARIES" "" ${ARGN})
    
    # get information about the data passed in, helpful for checking if a value
    # has been set or not
    LIST(LENGTH MMOSERVERLIB_DEBUG_LIBRARIES _debug_list_length)
    LIST(LENGTH MMOSERVERLIB_OPTIMIZED_LIBRARIES _optimized_list_length)
    LIST(LENGTH MMOSERVERLIB_MMOSERVER_DEPS _project_deps_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS _includes_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_SOURCE_DIRS _sources_list_length)
    
    # load up all of the source and header files for the project
    FILE(GLOB_RECURSE SOURCES *.cc *.cpp *.h)   
    FILE(GLOB_RECURSE TEST_SOURCES *_unittest.cc *_unittest.cpp mock_*.h)
        
    FOREACH(__source_file ${SOURCES})
        STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/)((.*/)*)(.*)" "\\2" __source_dir "${__source_file}")
        STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/${__source_dir})(.*)" "\\2" __source_filename "${__source_file}")
        
        STRING(REPLACE "/" "\\\\" __source_group "${__source_dir}")
        SOURCE_GROUP("${__source_group}" FILES ${__source_file})
        
        # check to see if this application specifies an explicit main file
        STRING(SUBSTRING ${__source_filename} 0 5 __main_check)
        STRING(COMPARE EQUAL "main." "${__main_check}" __is_main)
        IF(__is_main)
            SET(MAIN_EXISTS ${__source_file})
        ENDIF()        
    ENDFOREACH()
    
    # if unit tests have been specified break out the project into a library to make it testable
    LIST(LENGTH TEST_SOURCES _tests_list_length)    
    IF(_tests_list_length GREATER 0)        
        SET(__project_library "lib${name}")
    
        AddMMOServerLibrary(${__project_library}
            MMOSERVER_DEPS
                ${MMOSERVERLIB_MMOSERVER_DEPS}
            SOURCES
                ${SOURCES}
            TEST_SOURCES
                ${TEST_SOURCES}
            ADDITIONAL_INCLUDE_DIRS
                ${MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS}
            DEBUG_LIBRARIES
                ${MMOSERVERLIB_DEBUG_LIBRARIES}
            OPTIMIZED_LIBRARIES
                ${MMOSERVERLIB_OPTIMIZED_LIBRARIES}
        )
    
        set(SOURCES ${MAIN_EXISTS})
    ENDIF()
        
    IF(_includes_list_length GREATER 0)
        INCLUDE_DIRECTORIES(${MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS})
    ENDIF()
    
    # Set some default include directories for executables
    INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES})
    
    # Create the executable
    ADD_EXECUTABLE(${name} ${SOURCES})
    
    IF(_project_deps_list_length GREATER 0)
        TARGET_LINK_LIBRARIES(${name} ${MMOSERVERLIB_MMOSERVER_DEPS})
    ENDIF()
    
    IF(_debug_list_length GREATER 0)
        TARGET_LINK_LIBRARIES(${name} debug ${MMOSERVERLIB_DEBUG_LIBRARIES})
    ENDIF()
    
    IF(_optimized_list_length GREATER 0)
        TARGET_LINK_LIBRARIES(${name} optimized ${MMOSERVERLIB_OPTIMIZED_LIBRARIES})
    ENDIF()
    
    IF(WIN32)
        # Set the default output directory for binaries for convenience.
        SET_TARGET_PROPERTIES(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
        
        # Mysql is built with the static runtime but all of our projects and deps
        # use the dynamic runtime, in this instance it's a non-issue so ignore
        # the problem lib.
        SET_TARGET_PROPERTIES(${name} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
        
        # Link to some standard windows libs that all projects need.
    	TARGET_LINK_LIBRARIES(${name} "winmm.lib" "ws2_32.lib")
        
    	# After each executable project is built make sure the environment is
    	# properly set up (scripts, default configs, etc exist).
    	ADD_CUSTOM_COMMAND(TARGET ${name} POST_BUILD
            COMMAND call \"${PROJECT_SOURCE_DIR}/tools/windows/postbuild.bat\" \"${PROJECT_SOURCE_DIR}\" \"${PROJECT_BINARY_DIR}\" \"\$\(ConfigurationName\)\"
        )   
        
        # Create a custom built user configuration so that the "run in debug mode"
        # works without any issues.
    	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tools/windows/user_project.vcxproj.in 
    	    ${CMAKE_CURRENT_BINARY_DIR}/${name}.vcxproj.user @ONLY)
    ELSE()
        # On unix platforms put the built runtimes in the /bin directory.
        INSTALL(TARGETS ${name} RUNTIME DESTINATION bin)
    ENDIF()
        
    TARGET_LINK_LIBRARIES(${name}
        ${__project_library}
        ${MYSQL_LIBRARIES}
        debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
        debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG}
        debug ${Boost_REGEX_LIBRARY_DEBUG}
        debug ${Boost_SYSTEM_LIBRARY_DEBUG}
        debug ${Boost_THREAD_LIBRARY_DEBUG}
		debug ${Boost_LOG_LIBRARY_DEBUG}
        debug ${MysqlConnectorCpp_LIBRARY_DEBUG}
        debug ${TBB_LIBRARY_DEBUG}
        debug ${TBB_MALLOC_LIBRARY_DEBUG}
        debug ${ZLIB_LIBRARY_DEBUG}        
        optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
        optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        optimized ${Boost_REGEX_LIBRARY_RELEASE}
        optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
        optimized ${Boost_THREAD_LIBRARY_RELEASE}
		optimized ${Boost_LOG_LIBRARY_RELEASE}
        optimized ${MysqlConnectorCpp_LIBRARY_RELEASE}
        optimized ${TBB_LIBRARY}
        optimized ${TBB_MALLOC_LIBRARY}
        optimized ${ZLIB_LIBRARY_RELEASE}
    )
ENDFUNCTION()
