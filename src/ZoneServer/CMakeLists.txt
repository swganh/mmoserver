include(MMOServerExecutable)

set(ANH_PYTHON_BINDINGS "")

add_custom_target(
    swgpy ALL
    DEPENDS
		swgpy_app swgpy_utility
    COMMAND python "${CMAKE_SOURCE_DIR}/tools/swgpy/init_modules.py"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$\(Configuration\)/swgpy"
    VERBATIM
)

set_target_properties(swgpy
    PROPERTIES
    FOLDER "cmake_targets"
)


add_subdirectory(Services/terrain)

AddMMOServerExecutable(ZoneServer
    MMOSERVER_DEPS 
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager
        MessageLib
        SwgProtocol
    ADDITIONAL_INCLUDE_DIRS
        ${NOISE_INCLUDE_DIR} 
        ${PYTHON_INCLUDE_DIR}
    DEBUG_LIBRARIES
        ${LUA_LIBRARY_DEBUG}
        ${NOISE_LIBRARY_DEBUG}
        ${TOLUAPP_LIBRARY_DEBUG}
        ${Boost_PYTHON3_LIBRARY_DEBUG}
        ${PYTHON_LIBRARY}
        ${ZLIB_LIBRARY_DEBUG}
    OPTIMIZED_LIBRARIES
        ${LUA_LIBRARY_RELEASE}
        ${NOISE_LIBRARY_RELEASE}
        ${TOLUAPP_LIBRARY_RELEASE}
        ${Boost_PYTHON3_LIBRARY_RELEASE}
        ${PYTHON_LIBRARY}
	${ZLIB_LIBRARY_RELEASE}
)
