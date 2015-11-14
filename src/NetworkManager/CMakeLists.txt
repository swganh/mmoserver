include(MMOServerLibrary)

AddMMOServerLibrary(NetworkManager
    MMOSERVER_DEPS 
        Utils
        Common
    ADDITIONAL_INCLUDE_DIRS
        ${ZLIB_INCLUDE_DIR}
    DEBUG_LIBRARIES
        debug ${ZLIB_LIBRARY_DEBUG}       
    OPTIMIZED_LIBRARIES
        optimized ${ZLIB_LIBRARY_RELEASE}
)
TARGET_LINK_LIBRARIES(NetworkManager "-lrt -lpthread")