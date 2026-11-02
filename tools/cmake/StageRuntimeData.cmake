# Stages the data the servers load relative to their working directory next
# to the binaries, like build_server.sh does on Linux. Run after each build:
#
#   cmake -DSOURCE_DIR=<repo> -DBIN_DIR=<build>/bin -P StageRuntimeData.cmake

foreach(_var SOURCE_DIR BIN_DIR)
    if(NOT ${_var})
        message(FATAL_ERROR "StageRuntimeData: ${_var} is not set")
    endif()
endforeach()

file(MAKE_DIRECTORY "${BIN_DIR}/config" "${BIN_DIR}/logs")

# Keep existing configs: they hold local settings such as database passwords.
file(GLOB _configs "${SOURCE_DIR}/data/config/*.cfg")
foreach(_cfg IN LISTS _configs)
    get_filename_component(_name "${_cfg}" NAME)
    if(NOT EXISTS "${BIN_DIR}/config/${_name}")
        file(COPY "${_cfg}" DESTINATION "${BIN_DIR}/config")
    endif()
endforeach()

# Scripts are code; always refresh them.
file(REMOVE_RECURSE "${BIN_DIR}/script")
file(COPY "${SOURCE_DIR}/data/script" DESTINATION "${BIN_DIR}")

# Heightmaps are large; link to them instead of copying. A directory junction
# needs no administrator rights or developer mode, unlike a symbolic link.
if(EXISTS "${SOURCE_DIR}/data/heightmaps" AND NOT EXISTS "${BIN_DIR}/heightmaps")
    if(WIN32)
        file(TO_NATIVE_PATH "${BIN_DIR}/heightmaps" _link)
        file(TO_NATIVE_PATH "${SOURCE_DIR}/data/heightmaps" _target)
        execute_process(COMMAND cmd /c mklink /J "${_link}" "${_target}"
            OUTPUT_QUIET RESULT_VARIABLE _result)
        if(NOT _result EQUAL 0)
            message(WARNING "Could not link ${_link} to ${_target}")
        endif()
    else()
        file(CREATE_LINK "${SOURCE_DIR}/data/heightmaps" "${BIN_DIR}/heightmaps" SYMBOLIC)
    endif()
endif()
