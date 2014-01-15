include(MMOServerExecutable)

AddMMOServerExecutable(ConnectionServer
    MMOSERVER_DEPS 
        libanh
        Utils
        Common
        DatabaseManager
        NetworkManager
)
