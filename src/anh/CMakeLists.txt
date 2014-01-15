#python directories

include(MMOServerLibrary)

set(ANH_PYTHON_BINDINGS "")

#add_custom_target(
 #   swgpy ALL
  #  DEPENDS

#    COMMAND python "${CMAKE_SOURCE_DIR}/tools/swgpy/init_modules.py"
 #   WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$\(Configuration\)/swgpy"
  #  VERBATIM
#)

#set_target_properties(swgpy
#    PROPERTIES
#    FOLDER "cmake_targets"
#)


add_subdirectory(app)


AddMMOServerLibrary(libanh
    ADDITIONAL_INCLUDE_DIRS
        ${MYSQL_INCLUDE_DIR}
        ${MysqlConnectorCpp_INCLUDES}
        ${PYTHON_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
    DEBUG_LIBRARIES
        ${MYSQL_LIB}
        ${MysqlConnectorCpp_LIBRARY_DEBUG}
        ${PYTHON_LIBRARY}
        ${ZLIB_LIBRARY_DEBUG}
    OPTIMIZED_LIBRARIES
        ${MYSQL_LIB}
        ${MysqlConnectorCpp_LIBRARY_RELEASE}
        ${PYTHON_LIBRARY}
	${ZLIB_LIBRARY_RELEASE}
)
