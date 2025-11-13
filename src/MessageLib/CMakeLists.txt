include(MMOServerLibrary)

AddMMOServerLibrary(MessageLib
    MMOSERVER_DEPS 
        Utils
        Common
        NetworkManager
    ADDITIONAL_INCLUDE_DIRS
        ${NOISE_INCLUDE_DIR}
    DEBUG_LIBRARIES
        ${NOISE_LIBRARY_DEBUG}
    OPTIMIZED_LIBRARIES
        ${NOISE_LIBRARY_RELEASE}
)