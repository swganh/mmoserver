include(MMOServerLibrary)

AddMMOServerLibrary(NetworkManager
    MMOSERVER_DEPS
        Utils
        Common
    LIBRARIES
        ZLIB::ZLIB
)
