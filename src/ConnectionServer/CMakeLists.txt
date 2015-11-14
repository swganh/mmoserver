include(MMOServerExecutable)

AddMMOServerExecutable(ConnectionServer
    MMOSERVER_DEPS 
        DatabaseManager
        NetworkManager
        Common
        Utils
        )
TARGET_LINK_LIBRARIES(ConnectionServer "-lrt -lpthread")