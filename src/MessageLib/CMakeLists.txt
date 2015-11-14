include(MMOServerLibrary)

AddMMOServerLibrary(MessageLib
    MMOSERVER_DEPS 
        NetworkManager    
        Utils
        Common
    ADDITIONAL_INCLUDE_DIRS
        ${NOISE_INCLUDE_DIR}
    DEBUG_LIBRARIES
        ${NOISE_LIBRARY_DEBUG}
    OPTIMIZED_LIBRARIES
        ${NOISE_LIBRARY_RELEASE}
)
TARGET_LINK_LIBRARIES(MessageLib "-lrt -lpthread")