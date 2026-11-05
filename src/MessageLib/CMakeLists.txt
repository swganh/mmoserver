include(MMOServerLibrary)

AddMMOServerLibrary(MessageLib
    MMOSERVER_DEPS
        Utils
        Common
        NetworkManager
    LIBRARIES
        mmo::noise
)
