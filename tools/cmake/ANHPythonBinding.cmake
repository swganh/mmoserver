# AddANHPythonBinding is a standardized way to build python modules in the swganh
# project. Particularly on windows platforms this manages all the
# machinary to set up a default environment and creating/building/running unit
# unit tests an afterthought for developers.
#
# Function Definition:
#
# AddANHPythonBinding(library_name
#     DEPENDS [ARGS] [args1...]           	     # Dependencies on other swganh projects
#     ADDITIONAL_INCLUDE_DIRS [ARGS] [args1...]  # Additional directories to search for includes
#     ADDITIONAL_SOURCE_DIRS [ARGS] [args1...]   # Additional directories to search for files to include in the project
#     DEBUG_LIBRARIES [ARGS] [args1....]         # Additional debug libraries to link the project against
#     OPTIMIZED_LIBRARIES [ARGS] [args1...])     # Additional optimized libraries to link the project against
#
#
########################
# Simple Example Usage:
########################
#
# include(ANHPYTHONLIBrary)
#
# AddANHPythonBinding(Common
#     MMOSERVER_DEPS
#         Utils
# )
#
#
#########################
# Complex Example Usage:
#########################
# include(ANHPYTHONLIBrary)
#
# AddANHPythonBinding(ScriptEngine
#     DEPENDS
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

FUNCTION(AddANHPythonBinding name)
    PARSE_ARGUMENTS(ANHPYTHONLIB "PREFIX_NAME;POSTFIX_NAME;DEPENDS;SOURCES;ADDITIONAL_LIBRARY_DIRS;ADDITIONAL_INCLUDE_DIRS;ADDITIONAL_SOURCE_DIRS;DEBUG_LIBRARIES;OPTIMIZED_LIBRARIES" "" ${ARGN})

    set(base_name ${name})
    
    string(COMPARE NOTEQUAL "${ANHPYTHONLIB_PREFIX_NAME}" "" _has_prefix_name)
    if(${_has_prefix_name})
        set(PREFIX_NAME ${ANHPYTHONLIB_PREFIX_NAME}_)
    endif()
    
    string(COMPARE NOTEQUAL "${ANHPYTHONLIB_POSTFIX_NAME}" "" _has_postfix_name)
    if(${_has_postfix_name})
        set(POSTFIX_NAME _${ANHPYTHONLIB_POSTFIX_NAME})
    endif()
    set(name ${PREFIX_NAME}${name}${POSTFIX_NAME})

    LIST(LENGTH ANHPYTHONLIB_SOURCES __source_files_list_length)
    LIST(LENGTH ANHPYTHONLIB_DEBUG_LIBRARIES _debug_list_length)
    LIST(LENGTH ANHPYTHONLIB_OPTIMIZED_LIBRARIES _optimized_list_length)
    LIST(LENGTH ANHPYTHONLIB_DEPENDS _project_deps_list_length)
    LIST(LENGTH ANHPYTHONLIB_ADDITIONAL_INCLUDE_DIRS _includes_list_length)
    LIST(LENGTH ANHPYTHONLIB_ADDITIONAL_LIBRARY_DIRS _librarydirs_list_length)
    LIST(LENGTH ANHPYTHONLIB_ADDITIONAL_SOURCE_DIRS _sources_list_length)

    # Grab all of the source files and all of the unit test files.
    IF(__source_files_list_length EQUAL 0)
        # load up all of the source and header files for the project
        FILE(GLOB_RECURSE ANHPYTHONLIB_SOURCES *_binding.h *_binding.cc *_binding.cpp py_*.h py_*.cc)
		FOREACH(__source_file ${ANHPYTHONLIB_SOURCES})
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/)((.*/)*)(.*)" "\\2" __source_dir "${__source_file}")
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/${__source_dir})(.*)" "\\2" __source_filename "${__source_file}")

            STRING(REPLACE "/" "\\\\" __source_group "${__source_dir}")
            SOURCE_GROUP("${__source_group}" FILES ${__source_file})
#            SOURCE_GROUP("\\swgpy" FILES ${__source_file})
        ENDFOREACH()
    ELSE()
        FOREACH(__source_file ${ANHPYTHONLIB_SOURCES})
            STRING(REGEX REPLACE "(${CMAKE_CURRENT_SOURCE_DIR}/${__source_dir})(.*)" "\\2" __source_filename "${__source_file}")

            STRING(SUBSTRING ${__source_filename} 0 5 __main_check)
            STRING(COMPARE EQUAL "main." "${__main_check}" __is_main)
            IF(__is_main)
                LIST(REMOVE_ITEM ANHPYTHONLIB_SOURCES ${__source_file})
            ENDIF()
        ENDFOREACH()
    ENDIF()

    IF(_includes_list_length GREATER 0)
        INCLUDE_DIRECTORIES(${ANHPYTHONLIB_ADDITIONAL_INCLUDE_DIRS})
    ENDIF()

    IF(_librarydirs_list_length GREATER 0)
        LINK_DIRECTORIES(${ANHPYTHONLIB_ADDITIONAL_LIBRARY_DIRS})
    ENDIF()

    # Create the Common library
    ADD_LIBRARY(${name} MODULE ${ANHPYTHONLIB_SOURCES})

    if(WIN32)
        SET_TARGET_PROPERTIES(${name}
            PROPERTIES OUTPUT_NAME py_${base_name}
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/swgpy/${base_name}
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/swgpy/${base_name}
            PREFIX "../"
            SUFFIX ".pyd"
            FOLDER "python_bindings"
        )
    else()
        SET_TARGET_PROPERTIES(${name}
            PROPERTIES OUTPUT_NAME py_${base_name}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/swgpy/${base_name}
            PREFIX ""
            FOLDER "python_bindings"
        )
    endif()
	
    list(APPEND ANH_PYTHON_BINDINGS ${name})
	
    IF(_project_deps_list_length GREATER 0)
        ADD_DEPENDENCIES(${name} ${ANHPYTHONLIB_DEPENDS})
		TARGET_LINK_LIBRARIES(${name} ${ANHPYTHONLIB_DEPENDS})
    ENDIF()

    IF(_debug_list_length GREATER 0)
        FOREACH(debug_library ${ANHPYTHONLIB_DEBUG_LIBRARIES})
            if (NOT ${debug_library} MATCHES ".*NOTFOUND")
                TARGET_LINK_LIBRARIES(${name} debug ${debug_library})
            endif()
        ENDFOREACH()
    ENDIF()

    IF(_optimized_list_length GREATER 0)
        FOREACH(optimized_library ${ANHPYTHONLIB_OPTIMIZED_LIBRARIES})
            if (NOT ${optimized_library} MATCHES ".*NOTFOUND")
                TARGET_LINK_LIBRARIES(${name} optimized ${optimized_library})
            endif()
        ENDFOREACH()
    ENDIF()
ENDFUNCTION()
