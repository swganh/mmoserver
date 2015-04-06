include(MMOServerExecutable)

AddMMOServerExecutable(ConnectionServer
    MMOSERVER_DEPS 
        DatabaseManager
        NetworkManager
        Utils
        Common
)
