
string(REPLACE "/" "\\" WIN_PROJECT_BINARY_DIR "${PROJECT_BINARY_DIR}")
string(REPLACE "/" "\\" WIN_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

## create a target that gathers up the necessary dll's from the above
## dependencies and places them in the runtime directory for this project
add_custom_target(DEPS)

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WIN_PROJECT_BINARY_DIR}\\bin"
    VERBATIM
)

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)"
    VERBATIM
)

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)\\logs"
    VERBATIM
)

get_filename_component(GLOG_DLL_PATH ${GLOG_LIBRARY_DEBUG} PATH)
string(REPLACE "/" "\\" GLOG_DLL_PATH "${GLOG_DLL_PATH}\\..")
add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${GLOG_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)

get_filename_component(MYSQL_CONNECTOR_C_DLL_PATH ${MYSQL_LIB} PATH)
string(REPLACE "/" "\\" MYSQL_CONNECTOR_C_DLL_PATH "${MYSQL_CONNECTOR_C_DLL_PATH}\\..")
add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${MYSQL_CONNECTOR_C_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)


get_filename_component(MYSQL_CONNECTOR_CPP_DLL_PATH ${MYSQLCONNECTORCPP_LIBRARY_DEBUG} PATH)
string(REPLACE "/" "\\" MYSQL_CONNECTOR_CPP_DLL_PATH "${MYSQL_CONNECTOR_CPP_DLL_PATH}\\..")
add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${MYSQL_CONNECTOR_CPP_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)

get_filename_component(TBB_CPP_DLL_PATH ${TBB_LIBRARY_DEBUG} PATH)
string(REPLACE "/" "\\" TBB_CPP_DLL_PATH "${TBB_CPP_DLL_PATH}")
add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${TBB_CPP_DLL_PATH}\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)

get_filename_component(ZLIB_DLL_PATH ${ZLIB_LIBRARY_DEBUG} PATH)
string(REPLACE "/" "\\" ZLIB_DLL_PATH "${ZLIB_DLL_PATH}\\..")
add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${ZLIB_DLL_PATH}\\$\(Configuration\)\\*.dll" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)    

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${WIN_PROJECT_SOURCE_DIR}\\..\\data\\config" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)\\config" /D /I /Y /s
    VERBATIM
)  

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${WIN_PROJECT_SOURCE_DIR}\\..\\data\\script" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)\\script" /D /I /Y /s
    VERBATIM
)   

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${WIN_PROJECT_SOURCE_DIR}\\..\\data\\heightmaps\\*.hmpw" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)\\heightmaps" /D /I /Y /s
    VERBATIM
)    

add_custom_command(
    TARGET DEPS
    POST_BUILD
    COMMAND xcopy "${WIN_PROJECT_SOURCE_DIR}\\..\\data\\ServerStart.bat" "${WIN_PROJECT_BINARY_DIR}\\bin\\$\(Configuration\)" /D /I /Y /s
    VERBATIM
)
