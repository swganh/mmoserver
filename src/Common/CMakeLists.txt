include(MMOServerLibrary)

AddMMOServerLibrary(Common
    MMOSERVER_DEPS
        Utils
)

# The git revision for the login message (BuildInfo.cc), refreshed on every
# build.
set(BUILD_INFO_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/build_info_generated.h")
add_custom_target(build_info
    COMMAND "${CMAKE_COMMAND}" -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DOUTPUT=${BUILD_INFO_HEADER}
            -P "${PROJECT_SOURCE_DIR}/tools/cmake/BuildInfo.cmake"
    BYPRODUCTS "${BUILD_INFO_HEADER}"
    COMMENT "Build info from git")
add_dependencies(Common build_info)
target_include_directories(Common PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
