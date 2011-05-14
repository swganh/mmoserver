find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS
        $ENV{GLM_ROOT}
    PATH_SUFFIXES include
    PATHS
        ${GLM_ROOT}
        ${GLM_INCLUDEDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glm DEFAULT_MSG GLM_INCLUDE_DIR)

mark_as_advanced(GLM_INCLUDE_DIR)
