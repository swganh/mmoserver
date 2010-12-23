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

FUNCTION(AddMMOServerExecutable name)
    PARSE_ARGUMENTS(MMOSERVERLIB "MMOSERVER_DEPS;ADDITIONAL_INCLUDE_DIRS;ADDITIONAL_SOURCE_DIRS;DEBUG_LIBRARIES;OPTIMIZED_LIBRARIES" "" ${ARGN})
    
    # Get information about the data passed in, helpful for checking if a value
    # has been set or not.
    LIST(LENGTH MMOSERVERLIB_DEBUG_LIBRARIES _debug_list_length)
    LIST(LENGTH MMOSERVERLIB_OPTIMIZED_LIBRARIES _optimized_list_length)
    LIST(LENGTH MMOSERVERLIB_MMOSERVER_DEPS _project_deps_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS _includes_list_length)
    LIST(LENGTH MMOSERVERLIB_ADDITIONAL_SOURCE_DIRS _sources_list_length)
    
    # Load up all of the source and header files for the project.
    FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)   
    FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)       
    
    SOURCE_GROUP("" FILES ${SOURCES} ${HEADERS})
    
    IF(_sources_list_length GREATER 0)
        FOREACH(_source_dir ${MMOSERVERLIB_ADDITIONAL_SOURCE_DIRS})
            FILE(GLOB ADDITIONAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${_source_dir}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/${_source_dir}/*.cpp)
            FILE(GLOB ADDITIONAL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${_source_dir}/*.h)
            SOURCE_GROUP(${_source_dir} FILES ${ADDITIONAL_SOURCES} ${ADDITIONAL_HEADERS})
            LIST(APPEND SOURCES ${ADDITIONAL_SOURCES})
            LIST(APPEND HEADERS ${ADDITIONAL_HEADERS})
        ENDFOREACH()
    ENDIF()
    
    IF(_includes_list_length GREATER 0)
        INCLUDE_DIRECTORIES(${MMOSERVERLIB_ADDITIONAL_INCLUDE_DIRS})
    ENDIF()
    
    # Set some default include directories for executables
    INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${MysqlConnectorCpp_INCLUDES})
    
    # Create the executable
    ADD_EXECUTABLE(${name} ${SOURCES} ${HEADERS})   
    
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
        NetworkManager
        DatabaseManager
        Common
        Utils
        libanh
        debug ${Boost_DATE_TIME_LIBRARY_DEBUG}
        debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG}
        debug ${Boost_REGEX_LIBRARY_DEBUG}
        debug ${Boost_SYSTEM_LIBRARY_DEBUG}
        debug ${Boost_THREAD_LIBRARY_DEBUG}
        debug ${GLOG_LIBRARY_DEBUG}
        debug ${MysqlConnectorCpp_LIBRARY_DEBUG}
        debug ${TBB_LIBRARY_DEBUG}
        debug ${TBB_MALLOC_LIBRARY_DEBUG}
        debug ${ZLIB_LIBRARY_DEBUG}        
        optimized ${Boost_DATE_TIME_LIBRARY_RELEASE}
        optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        optimized ${Boost_REGEX_LIBRARY_RELEASE}
        optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
        optimized ${Boost_THREAD_LIBRARY_RELEASE}
        optimized ${GLOG_LIBRARY_RELEASE}
        optimized ${MysqlConnectorCpp_LIBRARY_RELEASE}
        optimized ${TBB_LIBRARY}
        optimized ${TBB_MALLOC_LIBRARY}
        optimized ${ZLIB_LIBRARY_RELEASE}
    )
ENDFUNCTION()
