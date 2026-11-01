include(MMOServerLibrary)

AddMMOServerLibrary(DatabaseManager
    MMOSERVER_DEPS
        Utils
        Common
    LIBRARIES
        mmo::mysql
)
