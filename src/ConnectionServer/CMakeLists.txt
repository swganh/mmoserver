include(MMOServerExecutable)

AddMMOServerExecutable(ConnectionServer
    MMOSERVER_DEPS 
        Utils
        Common
        DatabaseManager
        NetworkManager
)
